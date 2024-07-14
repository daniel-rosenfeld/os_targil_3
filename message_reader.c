#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char *argv[])
{
    char buffer[128];
    if (argc != 3)
    {
        printf("not the right amount of arguments");
        return -1;
    }
    int fd = open(argv[1], O_RDWR);
    if (fd = -1)
    {
        perror("message reader");
        return -1;
    }
    if (ioctl(fd, argv[2]) == -1)
    {
        perror("message reader");
        return -1;
    }
    if (read(fd, buffer, 128) == -1)
    {
        perror("message reader");
        return -1;
    }
    if (close(fd) == -1)
    {
        perror("message reader");
        return -1;
    }
    return 0;
}