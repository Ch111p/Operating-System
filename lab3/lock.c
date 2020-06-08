#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <stdarg.h>

#define PLATENUM 5

sem_t writeLock, spaceNum, printLock, readLock;
unsigned int readNum = 0, putNum = 0;
char plate[PLATENUM] = {0, 0, 0, 0, 0};

void init()
{
    srand(time(NULL));
    sem_init(&writeLock, 0, 1);
    sem_init(&spaceNum, 0, 5);
    sem_init(&printLock, 0, 1);
    sem_init(&readLock, 0, 1);
}

static char sprintf_buf[1024];

void print(char *fmt, ...)
{
    sem_wait(&printLock);
    va_list args;
    int n;
    va_start(args, fmt);
    n = vsprintf(sprintf_buf, fmt, args);
    va_end(args);
    write(STDOUT_FILENO, sprintf_buf, n);
    sem_post(&printLock);
}

void printStatus()
{
    print("|%c|%c|%c|%c|%c|\n", plate[0], plate[1], plate[2], plate[3], plate[4]);
}

void daughter()
{   
    while (1)
    {
        int val;
        while(sem_getvalue(&writeLock, &val), !val);
        sem_wait(&readLock);
        readNum++;
        sem_post(&readLock);
        for(int i = 0; i < PLATENUM; i++)
        {
            if(plate[i] == 'A')
            {
                print("Daughter eat one apple!\n");
                sem_post(&spaceNum);
                plate[i] = '\x00';
                break;
            }
            if(i == PLATENUM - 1)
            {
                print("No apple for daughter!\n");
            }
        }
        printStatus();
        sem_wait(&readLock);
        readNum--;
        sem_post(&readLock);
        if(!readNum)
        {
            sem_post(&writeLock);
        }            
    }
}

void son()
{
    while (1)
    {
        int val;
        while(sem_getvalue(&writeLock, &val), !val);
        sem_wait(&readLock);
        readNum++;
        sem_post(&readLock);
        for(int i = 0; i < PLATENUM; i++)
        {
            if(plate[i] == 'O')
            {
                print("Son eat one orange!\n");
                sem_post(&spaceNum);
                plate[i] = '\x00';
                break;
            }
            if(i == PLATENUM - 1)
            {
                print("No orange for son!\n");
            }
        }
        printStatus();
        sem_wait(&readLock);
        readNum--;
        sem_post(&readLock);
        if(!readNum)
        {
            sem_post(&writeLock);
        }            
    }
}

void father()
{
    while (1)
    {
        sem_wait(&writeLock);
        if(!putNum)
        { 
            print("END!\n");
            print("Final status is:\n");
            printStatus();
            exit(1);
        }
        if(sem_trywait(&spaceNum) == -1)
        {
            print("It's full!\n");
            printStatus();
            sem_post(&writeLock);
            continue;
        }
        for(int i = 0; i < PLATENUM; i++)
        {
            if(plate[i] == '\x00')
            {
                plate[i] = "AO"[rand() % 2];
                print("%d: Father put a %c into plate!\n", putNum, plate[i]);
                putNum--;
                break;
            }
        }
        printStatus();
        sem_post(&writeLock);
    }
}

int main()
{
    init();
    print("Please input scheduling count(min num is 11)\n");
    scanf("%u", &putNum);
    if(putNum < 11)
    {
        putNum += 11;
    }

    pthread_t Father, Son, Daughter;
    pthread_create(&Father, NULL, father, NULL);
    pthread_create(&Son, NULL, son, NULL);
    pthread_create(&Daughter, NULL, daughter, NULL);

    pthread_join(Father, NULL);
    pthread_join(Son, NULL);
    pthread_join(Daughter, NULL);

    return 0;
}
