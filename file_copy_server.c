#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>

#define DESTINATION_FILE    "destinaion_file"
#define PORT                8080

static int buf_size = 64; /* Default 64 bytes */

void usage(void)
{
    printf("Usage\n");
    printf("  file_copy_server -b <buffer size>\n\n");
    printf("Buffer size: bytes, default is 64\n");
}

int main(int argc, char **argv)
{
    int cmd_option;

    while ((cmd_option = getopt(argc, argv, "b:h")) != -1) {
        switch (cmd_option) {
            case 'b':
                sscanf(optarg, "%d", &buf_size);
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    struct sockaddr_in client_info = {0};
    struct sockaddr_in server_info = {0};
    socklen_t client_info_len = sizeof(client_info);
    int client_sockfd;
    int server_sockfd;
    int fd;
    char buf[buf_size];
    ssize_t len;

    if ((fd = open(DESTINATION_FILE,
                   O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        exit(1);
    }

    if ((server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket fail, errno: %d\n", errno);
        close(fd);
        exit(1);
    }

    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = htonl(INADDR_ANY);
    server_info.sin_port = htons(PORT);

    if (bind(server_sockfd, (struct sockaddr *)&server_info,
             sizeof(server_info)) == -1) {
        printf("bind fail, errno: %d\n", errno);
        close(fd);
        close(server_sockfd);
        exit(1);
    }

    if (listen(server_sockfd, 5) == -1) {
        printf("listen fail, errno: %d\n", errno);
        close(fd);
        close(server_sockfd);
        exit(1);
    }

    if ((client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_info,
                                &client_info_len)) == -1) {
        printf("accept fail, errno: %d\n", errno);
        close(fd);
        close(server_sockfd);
        exit(1);
    }

    while ((len = read(client_sockfd, buf, sizeof(buf))))
        write(fd, buf, len);

    close(fd);
    close(client_sockfd);
    close(server_sockfd);

    return 0;
}