#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>
#include <time.h>

char* tools[] = {"ls", "pwd", "rm"};

void exprBuf(char *buf, char *cmd, char **arg)
{
    int i = 0;
    while(++i)
    {
        char *temp = strchr(buf, ' ');
        if(!temp)
        {
            break;
        }
        if(i == 1)
        {
            strncpy(cmd, buf, temp - buf);
        }
        else
        {
            arg[i - 1] = strndup(buf, temp - buf);
        }
        buf = temp + 1;
    }
    printf("%s\n", buf);
    i==1?(strcpy(cmd, buf)):(arg[i - 1] = strdup(buf));
}

int main(int argc, char *argv[])
{
    while(1)
    {
        char tempBuf[1024] = {0, };
        char *arg[1024] = {NULL, };
        char cmd[1024] = {0, };
        printf("> ");
        fgets(tempBuf, 1024, stdin);
        tempBuf[strlen(tempBuf)-1] = '\x00';
        if(!strcmp(tempBuf, "exit"))
        {
            printf("Bye\n");
            break;
        }
        exprBuf(tempBuf, cmd, arg);
        for(int i = 0; i < (sizeof(tools) / sizeof(char*)); i++)
        {
            if(!strcmp(cmd, tools[i]))
            {
                __pid_t tempPID = fork();
                if(tempPID == 0)
                {
                    char tempA[4096] = {0, };
                    getcwd(tempA, 4096);
                    tempA[strlen(tempA)] = '/';
                    strncat(tempA, tools[i], 4096);
                    arg[0] = strdup(tempA);
                    int ret = execv(tools[i], arg);
                    exit(ret);
                }
                else
                {
                    wait();
                    break;
                }
            }
            if(i == (sizeof(tools) / sizeof(char*) - 1))
            {
                printf("No such cmd!\n");
            }
        }
    }
    return 0;
}