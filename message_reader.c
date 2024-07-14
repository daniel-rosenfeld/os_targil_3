#include <sys/ioctl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include "message_slot.h"
#include <stdlib.h>

int main(int argc, char *argv[])
{
    char buffer[128];
    for (int i = 0; i < 128; i++){
        buffer[i] = '\0';
    }
    if (argc != 3)
    {
        printf("not the right amount of arguments");
        return -1;
    }
    int fd = open(argv[1], O_RDWR);
    if (fd < 0)
    {
        perror("message reader");
        return -1;
    }
    if (ioctl(fd, MSG_SLOT_CHANNEL, atoi(argv[2])) < 0)
    {
        perror("message reader");
        return -1;
    }
    if (read(fd, buffer, 128) < 0)
    {
        perror("message reader");
        return -1;
    }
    if (close(fd) < 0)
    {
        perror("message reader");
        return -1;
    }
    write(STDOUT_FILENO, buffer, sizeof(buffer));
    return 0;
}