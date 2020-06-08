#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>

#define BUFLENGTH 4096

void alert(char *message)
{
    printf("%s\n", strerror(errno));
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

int main()
{
    int tempFD;
    char buf[BUFLENGTH] = {0, };
    while((tempFD = shm_open("/temp", O_RDWR, 0777)) == -1);
    printf("%d\n", tempFD);
    sem_t *lockW = sem_open("/LOCKW", O_EXCL);
    sem_t *lockR = sem_open("/LOCKR", O_EXCL);
    ftruncate(tempFD, BUFLENGTH);
    void *addr = mmap(NULL, BUFLENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, tempFD, 0);
    if(addr == (void*)-1)
    {
        alert("failed when mmap");
    }
    do
    {
        sem_wait(lockR);
        strcpy(buf, addr);
        printf("[RECIEVER]: %s\n", buf);
        sem_post(lockW);
    } while (strcmp(buf, "end"));
    strcpy(addr, "over");
    sem_post(lockW);
    munmap(addr, BUFLENGTH);
    printf("Reciever end\n");
    return 0;
}