#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <semaphore.h>
#include <errno.h>
#include <stdbool.h>

#define MAX_BUF_LENGTH 1024
#define QUEUE_MAXMSG  16 /* Maximum number of messages. */
#define QUEUE_MSGSIZE 1024 /* Length of message. */
#define QUEUE_ATTR_INITIALIZER ((struct mq_attr){0, QUEUE_MAXMSG, QUEUE_MSGSIZE, 0, {0}})

sem_t tempSem1, tempSem2;

int messageNum = 0;
bool writing = false;

// int my_mq_send(mqd_t mq1, char *buf, sem_t *sem)
// {
//     sem_wait(sem);
//     messageNum++;
//     int retVal = mq_send(mq1, buf, strlen(buf) + 1, 0);
//     if(retVal < 0)
//     {
//         printf("%s\n", strerror(errno));
//     }
//     sem_post(sem);
//     return retVal;
// }

// int my_mq_receive(mqd_t mq1, char *buf, sem_t *sem)
// {
//     sem_wait(sem);
//     messageNum--;
//     unsigned int temp;
//     int retVal = mq_receive(mq1, buf, MAX_BUF_LENGTH, &temp);
//     if(retVal < 0)
//     {
//         printf("%s\n", strerror(errno));
//     }
//     sem_post(sem);
//     return retVal;
// }

void* sender1(void *temp)
{
    mqd_t mq1;
    struct mq_attr attr = QUEUE_ATTR_INITIALIZER;
    char buf[MAX_BUF_LENGTH] = {0, };
    mq1 = mq_open("/mqueue", O_RDWR | O_CREAT, 0644, &attr);
    printf("Create %x", mq1);
    printf("Now it's in sender1, Please input whatever your want.\n");
    while (1)
    {
        sem_wait(&tempSem1);
        while(writing);
        if(messageNum)
        {
            sem_post(&tempSem1);
            continue;
        }
        scanf("%1024s", buf);
        if(!strcmp(buf, "exit")) break;
        printf("sender1:%s\n", buf);
        messageNum++;
        sem_post(&tempSem1);
        mq_send(mq1, buf, strlen(buf) + 1, 0);
    }
    mq_send(mq1, "end1", 5, 0);
    messageNum++;
    sem_post(&tempSem1);
    while(!writing);
    sem_wait(&tempSem1);
    mq_receive(mq1, buf, MAX_BUF_LENGTH, 0);
    printf("sender1 receive message:%s, now quit\n", buf);
    mq_close(mq1);
    return NULL;
}

void* sender2(void *temp)
{
    mqd_t mq1;
    char buf[MAX_BUF_LENGTH] = {0, };
    while((mq1 = mq_open("/mqueue", O_RDWR)) == -1);
    printf("Now it's in sender2, Please input whatever your want.\n");
    while (1)
    {
        sem_wait(&tempSem2);
        while(writing);
        if(messageNum)
        {
            sem_post(&tempSem2);
            continue;
        }
        scanf("%1024s", buf);
        if(!strcmp(buf, "exit")) break;
        printf("sender2:%s\n", buf);
        messageNum++;
        sem_post(&tempSem2);
        mq_send(mq1, buf, strlen(buf) + 1, 0);
    }
    mq_send(mq1, "end2", 5, 0);
    messageNum++;
    sem_post(&tempSem2);
    while(!writing);
    sem_wait(&tempSem2);
    mq_receive(mq1, buf, MAX_BUF_LENGTH, 0);
    printf("sender2 receive message:%s, now quit\n", buf);
    mq_close(mq1);
    return NULL;
}

void* receiver(void *temp)
{
    mqd_t mq1;
    int status = 0;
    int tempPrio;
    char buf[MAX_BUF_LENGTH] = {0, };
    while((mq1 = mq_open("/mqueue", O_RDWR)) == -1);
    printf("Now it's in reciver\n");
    while(1)
    {
        if(status == 3)
        {
            sem_close(&tempSem1);
            sem_close(&tempSem2);
            mq_unlink("/mqueue");
            break;
        }
        if(messageNum)
        {
            if(status != 1 && (sem_trywait(&tempSem1) != -1))
            {
                writing = true;
                messageNum--;
                mq_receive(mq1, buf, 1024, &tempPrio);
                printf("[Sender1]:%s\n", buf);
                if(!strcmp(buf, "end1"))
                {
                    mq_send(mq1, "over1", 5, 0);
                    printf("here1, messageNum:%d\n", messageNum);
                    status += 1;
                }
                sem_post(&tempSem1);
            }
            else if(status != 2 && (sem_trywait(&tempSem2) != -1))
            {
                writing = true;
                messageNum--;
                mq_receive(mq1, buf, 1024, &tempPrio);
                printf("[Sender2]:%s\n", buf);
                if(!strcmp(buf, "end2"))
                {
                    mq_send(mq1, "over2", 5, 0);
                    printf("here2, messageNum:%d\n", messageNum);
                    status += 2;
                }
                sem_post(&tempSem2);            
            }
            writing = false;
        }
    }
    pthread_exit(NULL);
}

int main()
{
    pthread_attr_t attr[3];
    pthread_t thread[3];
    void* (*func[3])(void*) = {sender1, sender2, receiver};
    void *ret[3];
    sem_init(&tempSem1, 0, 1);
    sem_init(&tempSem2, 0, 1);
    for(int i = 0; i < 3; i++)
    {
        pthread_attr_init(&attr[i]);
        pthread_create(&thread[i], &attr[i], func[i], NULL);
    }
    for(int i = 0; i < 3; i++)
    {
        pthread_join(thread[i], &ret[i]);
    }
    return 0;
}