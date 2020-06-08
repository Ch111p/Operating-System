#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <limits.h>

int main()
{
    char tempA[4096] = {0, };
    getcwd(tempA, 4096);
    printf("%s\n", tempA);
    DIR *dp = opendir(tempA);
    struct dirent *temp;
    while(temp=readdir(dp))
    {
        printf("%s\n", temp->d_name);
    }
    closedir(dp);
    return 0;
}