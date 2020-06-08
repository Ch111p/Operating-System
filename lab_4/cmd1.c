#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    char tempA[4096];
    getcwd(tempA, 4096);
    puts(tempA);
    return 0;
}