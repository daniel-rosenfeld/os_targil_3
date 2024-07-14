#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("not the right amount of arguments");
        return -1;
    }
    int fd = open(argv[1], O_RDWR);
    if (fd = -1)
    {
        perror("message_sender");
        return -1;
    }
    if (ioctl(fd, argv[2]) == -1)
    {
        perror("message_sender");
        return -1;
    }
    if (write(fd, argv[3], strlen(argv[3])) == -1)
    {
        perror("message_sender");
        return -1;
    }
    if (close(fd) == -1)
    {
        perror("message_sender");
        return -1;
    }
    return 0;
}