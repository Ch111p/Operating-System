#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>
#include <string.h>

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        printf("usage: rm filename\n");
        return -1;
    }
    char tempA[4096] = {0, };
    getcwd(tempA, 4096);
    DIR *dp = opendir(tempA);
    struct dirent *temp;
    while(temp=readdir(dp))
    {
        char *fileName = temp->d_name;
        if(!strcmp(fileName, argv[1]))
        {
            remove(temp->d_name);
            break;
        }
    }
    return 0;
}