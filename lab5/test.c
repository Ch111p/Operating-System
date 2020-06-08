#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <fcntl.h>

typedef struct argType
{
    int start;
    int length;
    char *mem;
}arg, *pArg;

char *temp = "testtest";

int main()
{
    char ret[6] = {0, };
    arg tempArg = {0, 9, ret};
    int fd = open("/dev/char_device4", O_RDWR);
    printf("Now get %d\n", fd);
    write(fd, temp, strlen(temp) + 1);
    ioctl(fd, 1, &tempArg);
    printf("get %s\n", tempArg.mem);
    close(fd);
    return 0;
}