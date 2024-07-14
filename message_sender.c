#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "message_slot.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("not the right amount of arguments");
        return -1;
    }
    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("message_sender");
        return -1;
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2])) < 0)
    {
        perror("message_sender");
        return -1;
    }
    if (write(fd, argv[3], strlen(argv[3])) < 0)
    {
        perror("message_sender");
        return -1;
    }
    if (close(fd) < 0)
    {
        perror("message_sender");
        return -1;
    }
    return 0;
}