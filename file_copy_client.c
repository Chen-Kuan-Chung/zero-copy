#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>

#define SOURCE_FILE         "source_file"
#define MODE_READ_WRITE     "1"
#define MODE_MMAP_WRITE     "2"
#define MODE_SENDFILE       "3"
#define PORT                8080
#define IPV4_ADDR_LEN       16

enum file_copy_mode {
    mode_read_write,
    mode_mmap_write,
    mode_sendfile,
    mode_unknown
};

static struct timeval start;
static struct timeval end;
static struct timeval diff;
static int buf_size = 64; /* Default 64 bytes */
static int sockfd = 0;
static char *buf = NULL;
static char ip[IPV4_ADDR_LEN] = "127.0.0.1"; /* Default ip 127.0.0.1 */

void usage(void)
{
    printf("Usage\n");
    printf("  file_copy -i <ip> -m <mode> -b <buffer size>\n\n");
    printf("IP: Server IP address, default is 127.0.0.1\n\n");
    printf("Mode:\n");
    printf("  1:        read/write\n");
    printf("  2:        mmap/write\n");
    printf("  3:        sendfile\n\n");
    printf("Buffer size: bytes, default is 64\n");
}

void file_copy_read_write(void)
{
    int fd;
    char buf[buf_size];
    ssize_t len;

    if ((fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(sockfd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    while ((len = read(fd, buf, sizeof(buf))))
        write(sockfd, buf, sizeof(buf));
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("read/write = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

    close(fd);
    close(sockfd);
}

void file_copy_mmap_write(void)
{
    struct stat statbuf;
    int fd;
    int i = 0;
    char *addr;

    if ((fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(sockfd);
        exit(1);
    }

    if (fstat(fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(fd);
        close(sockfd);
        exit(1);
    }

    addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if (addr == MAP_FAILED){
        printf("mmap fail, errno: %d\n", errno);
        close(fd);
        close(sockfd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    while (i < (statbuf.st_size / buf_size)) {
        write(sockfd, addr + (i * buf_size), buf_size);
        i++;
    }
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("mmap/write = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

    munmap(addr, statbuf.st_size);
    close(fd);
    close(sockfd);
}

void file_copy_sendfile(void)
{
    struct stat statbuf;
    int fd;

    if ((fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(sockfd);
        exit(1);
    }

    if (fstat(fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(fd);
        close(sockfd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    sendfile(sockfd, fd, 0, statbuf.st_size);
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("sendfile = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

    close(fd);
    close(sockfd);
}

int main(int argc, char **argv)
{
    int cmd_option;
    int mode = mode_unknown;

    while ((cmd_option = getopt(argc, argv, "i:m:b:h")) != -1) {
        switch (cmd_option) {
            case 'i':
                strncpy(ip, optarg, IPV4_ADDR_LEN);
            case 'm':
                if (!strcmp(optarg, MODE_READ_WRITE))
                    mode = mode_read_write;
                else if (!strcmp(optarg, MODE_MMAP_WRITE))
                    mode = mode_mmap_write;
                else if (!strcmp(optarg, MODE_SENDFILE))
                    mode = mode_sendfile;
                else
                    mode = mode_unknown;
                break;
            case 'b':
                sscanf(optarg, "%d", &buf_size);
                break;
            case 'h':
            default:
                usage();
                return 0;
        }
    }

    struct sockaddr_in server_info = {0};
    
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("socket fail, errno: %d\n", errno);
        exit(1);
    }

    server_info.sin_family = AF_INET;
    server_info.sin_addr.s_addr = inet_addr(ip);
    server_info.sin_port = htons(PORT);

    if (connect(sockfd, (struct sockaddr *)&server_info,
                sizeof(server_info)) == -1) {
        printf("connect fail, errno: %d\n", errno);
        close(sockfd);
        exit(1);
    }

    if (mode == mode_read_write)
        file_copy_read_write();
    else if (mode == mode_mmap_write)
        file_copy_mmap_write();
    else if (mode == mode_sendfile)
        file_copy_sendfile();
    else
        printf("Unknown mode\n");

    return 0;
}