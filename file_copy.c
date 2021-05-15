#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>

#define BUF_SIZE            64
#define SOURCE_FILE         "source_file"
#define DESTINATION_FILE    "destinaion_file"
#define MODE_READ_WRITE     "1"
#define MODE_MMAP_WRITE     "2"
#define MODE_SENDFILE       "3"

void usage(void)
{
    printf("Usage\n");
    printf("  file_copy -m <mode>\n\n");
    printf("Mode:\n");
    printf("  1:    read/write\n");
    printf("  2:    mmap/write\n");
    printf("  3:    sendfile\n");
}

void file_copy_read_write(void)
{
    int write_fd;
    int read_fd;
    char buf[BUF_SIZE];
    ssize_t len;

    if ((read_fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        return ;
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        return ;
    }

    while ((len = read(read_fd, buf, sizeof(buf))))
        write(write_fd, buf, sizeof(buf));

    close(read_fd);
    close(write_fd);
}

void file_copy_mmap_write(void)
{
    struct stat statbuf;
    int write_fd;
    int read_fd;
    int i = 0;
    char *addr;

    if ((read_fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        return ;
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        return ;
    }

    if (fstat(read_fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(read_fd);
        close(write_fd);
        return ;
    }

    addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, read_fd, 0);
    if (addr == MAP_FAILED){
        printf("mmap() fail, errno: %d\n", errno);
        close(read_fd);
        close(write_fd);
        return ;
    }

    while (i < (statbuf.st_size / BUF_SIZE)) {
        write(write_fd, addr + (i * BUF_SIZE), BUF_SIZE);
        i++;
    }

    munmap(addr, statbuf.st_size);
    close(read_fd);
    close(write_fd);
}

void file_copy_sendfile(void)
{
    struct stat statbuf;
    int write_fd;
    int read_fd;

    if ((read_fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        return ;
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        return ;
    }

    if (fstat(read_fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(read_fd);
        close(write_fd);
        return ;
    }

    sendfile(write_fd, read_fd, 0, statbuf.st_size);

    close(read_fd);
    close(write_fd);
}

int main(int argc, char **argv)
{
    if (argc == 3 && !strcmp(argv[1], "-m")) {
        if (!strcmp(argv[2], MODE_READ_WRITE))
            file_copy_read_write();
        else if (!strcmp(argv[2], MODE_MMAP_WRITE))
            file_copy_mmap_write();
        else if (!strcmp(argv[2], MODE_SENDFILE))
            file_copy_sendfile();
        else
            printf("Unknow mode\n");
    } else {
        usage();
    }

    return 0;
}