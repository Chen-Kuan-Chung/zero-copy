#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/time.h>

#define SOURCE_FILE         "source_file"
#define DESTINATION_FILE    "destinaion_file"
#define MODE_READ_WRITE     "1"
#define MODE_MMAP_WRITE     "2"
#define MODE_SENDFILE       "3"

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
static char *buf = NULL;

void usage(void)
{
    printf("Usage\n");
    printf("  file_copy -m <mode> -b <buffer size>\n\n");
    printf("Mode:\n");
    printf("  1:        read/write\n");
    printf("  2:        mmap/write\n");
    printf("  3:        sendfile\n\n");
    printf("Buffer size: bytes, default is 64\n");
}

void file_copy_read_write(void)
{
    int write_fd;
    int read_fd;
    char buf[buf_size];
    ssize_t len;

    if ((read_fd = open(SOURCE_FILE, O_RDONLY)) == -1) {
        printf("open file %s fail, errno: %d\n", SOURCE_FILE, errno);
        exit(1);
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    while ((len = read(read_fd, buf, sizeof(buf))))
        write(write_fd, buf, sizeof(buf));
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("read/write = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

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
        exit(1);
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        exit(1);
    }

    if (fstat(read_fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(read_fd);
        close(write_fd);
        exit(1);
    }

    addr = mmap(NULL, statbuf.st_size, PROT_READ, MAP_SHARED, read_fd, 0);
    if (addr == MAP_FAILED){
        printf("mmap fail, errno: %d\n", errno);
        close(read_fd);
        close(write_fd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    while (i < (statbuf.st_size / buf_size)) {
        write(write_fd, addr + (i * buf_size), buf_size);
        i++;
    }
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("mmap/write = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

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
        exit(1);
    }

    if ((write_fd = open(DESTINATION_FILE,
                         O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
        printf("open file %s fail, errno: %d\n", DESTINATION_FILE, errno);
        close(read_fd);
        exit(1);
    }

    if (fstat(read_fd, &statbuf) == -1) {
        printf("fstat file %s fail, errno: %d\n", SOURCE_FILE, errno);
        close(read_fd);
        close(write_fd);
        exit(1);
    }

    gettimeofday(&start, NULL);
    sendfile(write_fd, read_fd, 0, statbuf.st_size);
    gettimeofday(&end, NULL);

    timersub(&end, &start, &diff);

    printf("sendfile = %0.3f sec\n", diff.tv_sec + (double) diff.tv_usec / 1000000.0);

    close(read_fd);
    close(write_fd);
}

int main(int argc, char **argv)
{
    int cmd_option;
    int mode = mode_unknown;

    while ((cmd_option = getopt(argc, argv, "m:b:h")) != -1) {
        switch (cmd_option) {
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