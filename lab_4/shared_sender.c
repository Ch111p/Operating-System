#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>

#define BUFLENGTH 4096

void alert(char *message)
{
    printf("%s\n", message);
    exit(EXIT_FAILURE);
}

int main()
{
    char buf[BUFLENGTH] = {0, };
    sem_t *lockW = sem_open("/LOCKW", O_CREAT, 0777, 0);
    sem_t *lockR = sem_open("/LOCKR", O_CREAT, 0777, 0);
    int tempFD = shm_open("/temp", O_CREAT | O_RDWR, 0777);
    if(tempFD == -1)
    {
        alert("failed when shm_open");
    }
    ftruncate(tempFD, BUFLENGTH);
    void *addr = mmap(NULL, BUFLENGTH, PROT_WRITE | PROT_READ, MAP_SHARED, tempFD, 0);
    if(addr == (void*)-1)
    {
        alert("failed when mmap");
    }
    printf("Please input whatever you want:\n");
    while (~scanf("%s", buf))
    {
        strcpy(addr, buf);
        sem_post(lockR);
        sem_wait(lockW);
        if(!strcmp(buf, "end")) break;
    }
    sem_wait(lockW);
    strcpy(buf, addr);
    printf("Sender recieve %s\n", buf);
    munmap(addr, BUFLENGTH);
    shm_unlink("/temp");
    sem_destroy(lockR);
    sem_destroy(lockW);
    printf("Sender is end\n");
    return 0;
}