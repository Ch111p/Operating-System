#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <fcntl.h>


int main(int argc, char *argv[])
{
    int pipefd[2];
    pid_t cpid[3] = {-2. -2. -2};
    char buf;
    char temp = 'a';
    unsigned int length = 0;

    if(pipe2(pipefd, O_NONBLOCK) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    sem_t *lockW = sem_open("/LOCKC", O_CREAT | O_EXCL, 0777, 1);
    if(lockW < 0)
    {
        perror("shm_open");
        exit(EXIT_FAILURE);
    }

    for(int i = 0; i < 3; i++)
    {
        if(i > 0 && cpid[i - 1] == 0)
        {
            break;
        }
        cpid[i] = fork();
        if(cpid[i] == -1)
        {
            perror("fork");
            exit(EXIT_FAILURE);
        }
    }
    
    if(!cpid[0] || !cpid[1] || !cpid[2])
    {
        int i = 1;
        close(pipefd[0]);
        sem_wait(lockW);
        printf("This is process %d, Now input!!\n", getpid());
        while(write(pipefd[1], &temp, 1) != -1)
        {
            i++;
        }
        printf("send %d char\n", i);
        sem_post(lockW);
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }
    else
    {
        close(pipefd[1]);
        for(int i = 0; wait(NULL) && i < 3; i++);
        int i = 1;
        while(read(pipefd[0], &temp, 1) && i++)
        {
            write(STDOUT_FILENO, &temp, 1);
        }
        printf("recv %d char\n", --i);
        close(pipefd[0]);
        sem_close(lockW);
        sem_unlink("/LOCKC");
        exit(EXIT_SUCCESS);
    }
}