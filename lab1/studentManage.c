/*
Author: 17051713
gcc studentManage.c -o test
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

typedef struct student
{
    int sno;
    char *sname;
    int sage;
    int sclass;
    struct student* next;
} stuInfo, *pStuInfo;

#define ADD 1
#define DEL 2
#define SEEA 3
#define SEEO 4
#define SORT 5
#define GG 6

pStuInfo findStudent(pStuInfo tempStu, int num)
{
    while(tempStu)
    {
        if(tempStu->sno == num)
        {
            return tempStu;
        }
        tempStu = tempStu->next;
    }
    return NULL;
}

int cmp(const void* a, const void* b)
{
    if( ((pStuInfo)a)->sclass != ((pStuInfo)b)->sclass )
    {
        return (((pStuInfo)a)->sclass - ((pStuInfo)b)->sclass);
    }
    if( ((pStuInfo)a)->sno != ((pStuInfo)b)->sno )
    {
        return (((pStuInfo)a)->sno - ((pStuInfo)b)->sno);
    }
    if( ((pStuInfo)a)->sage != ((pStuInfo)b)->sage )
    {
        return (((pStuInfo)a)->sage - ((pStuInfo)b)->sage);
    }
    if( ((pStuInfo)a)->sname[0] != ((pStuInfo)b)->sname[0] )
    {
        return (((pStuInfo)a)->sname[0] - ((pStuInfo)b)->sname[0]);
    }
    return 0;        
}

int getStudentNum(pStuInfo tempStu)
{
    int num = 0;
    while(num++, tempStu) tempStu = tempStu->next;
    return (num - 2);
}

pStuInfo initStudent(char *name, int num, int age, int class)
{
    pStuInfo temp = (pStuInfo)malloc(sizeof(stuInfo));
    if(!temp)
    {
        return false;
    }
    temp->sname = name;
    temp->sno = num;
    temp->sage = age;
    temp->sclass = class;
    temp->next = NULL;
    return temp;
}

bool printAllStudent(pStuInfo tempStu)
{
    if(!tempStu)
    {
        return false;
    }
    printf("Name\t\tClass\t\tAge\t\tNumber\n");
    while(tempStu->next)
    {
        printf("%s\t\t%d\t\t%d\t\t%d\n", \
            tempStu->next->sname, \
            tempStu->next->sclass, \
            tempStu->next->sage, \
            tempStu->next->sno);
        tempStu = tempStu->next;
    }
    return true;
}

bool printOneStudent(pStuInfo stu)
{
    if(!stu)
    {
        return false;
    }
    printf("Name\t\tClass\t\tAge\t\tNumber\n");
    printf("%s\t\t%d\t\t%d\t\t%d\n", \
            stu->sname, \
            stu->sclass, \
            stu->sage, \
            stu->sno);
    return true;
}

bool addStudent(pStuInfo tempStu, pStuInfo stu)
{
    if(!stu || !tempStu)
    {
        return false;
    }
    while(tempStu->next)
    {
        tempStu = tempStu->next;
    }
    tempStu->next = stu;
    return true;
}

bool delStudent(pStuInfo tempStu, int num)
{
    if(!tempStu)
    {
        return false;
    }
    while(tempStu)
    {
        if(tempStu->next && \
            tempStu->next->sno == num)
        {
            pStuInfo temp = tempStu->next->next;
            free(tempStu->next->sname);
            free(tempStu->next);
            tempStu->next = temp;
            return true;
        }
        printf("%d\n", num);
        tempStu = tempStu->next;
    }
    return false;
}

bool changeStudent(pStuInfo tempStu, pStuInfo stu)
{
    if(!tempStu || !stu)
    {
        return false;
    }
    while(tempStu)
    {
        if(tempStu->next && \
            tempStu->next->sno == stu->sno)
        {
            pStuInfo temp = tempStu->next;
            tempStu->next = stu;
            stu->next = temp->next;
            free(temp);
        }
        tempStu = tempStu->next;
    }
}

bool sortStudent(pStuInfo tempStu)
{
    if(!tempStu)
    {
        return false;
    }
    int num = getStudentNum(tempStu);
    pStuInfo head = tempStu;
    pStuInfo temp = (pStuInfo)malloc(sizeof(stuInfo) * num);
    for(int i = 0; i < num; i++)
    {
        memcpy(temp + i, tempStu->next, sizeof(stuInfo));
        temp[i].next = tempStu->next;
        tempStu = tempStu->next;
    }
    qsort(temp, num, sizeof(stuInfo), cmp);
    for(int i = 0; i < num; i++)
    {
        head->next = (pStuInfo)(temp[i]).next;
        head = head->next;
    }
    head->next = NULL;
    free(temp);
    return true;
}

void printMenu()
{
    puts("It's a easy--easy student manage system");
    puts("Please tell me what's you want to do");
    puts("1. add student");
    puts("2. del student");
    puts("3. see see students");
    puts("4. see one student");
    puts("5. sort it!");
    puts("6. bye bye");
    printf(">");
    fflush(stdout);
}

void print(char *str)
{
    printf("%s", str);
    fflush(stdout);
}

void getSnum(int *num)
{
    print("Please input Sno >");
    scanf("%d", num);
    fflush(stdin);
}

void deleteAll(pStuInfo tempStu)
{
    while(tempStu)
    {
        pStuInfo temp = tempStu;
        if(temp->sname)
        {
            free(tempStu->sname);
        }
        tempStu = tempStu->next;
        free(temp);
    }
}

void getAllMessage(int *num, int *age, char **name, int *class)
{
    print("Please input Sno >");
    scanf("%d", num);
    print("Please input Sname >");
    fflush(stdin);
    *name = (char*)malloc(0x100);
    scanf("%255s", *name);
    fflush(stdin);
    print("Please input class num >");
    scanf("%d", class);
    print("Please input age >");
    scanf("%d", age);
    puts("OK, now all finish");
}

void errHandle()
{
    puts("Something Error!");
    exit(-1);
}

int main()
{
    pStuInfo headStu = initStudent(NULL, -1, -1, -1);
    while(1)
    {
        unsigned int choice = 0;
        int Sno = 0;
        int Sage = 0;
        char *Sname = NULL;
        int Sclass = 0;
        pStuInfo temp = NULL;
        printMenu();
        scanf("%d", &choice);
        while(choice > 6 || choice < 1)
        {
            puts("Bad guy, retry!");
            scanf("%d", &choice);
        }
        switch (choice)
        {
        case ADD:
            getAllMessage(&Sno, &Sage, &Sname, &Sclass);
            if (temp = initStudent(Sname, Sno, Sage, Sclass))
            {
                if(!addStudent(headStu, temp))
                {
                    errHandle();
                }
                puts("OK!");
            }
            else
            {
                errHandle();
            }
            break;
        case DEL:
            getSnum(&Sno);
            if (!delStudent(headStu, Sno))
            {
                puts("No such guy");
            }
            else
            {
                puts("OK!");
            }
            break;
        case SEEO:
            getSnum(&Sno);
            temp = findStudent(headStu, Sno);
            if (temp)
            {
                if (!printOneStudent(temp))
                {
                    puts("Something error!");
                }
                else
                {
                    puts("Ok!");
                }
            }
            else
            {
                puts("No such people!");
            }
            break;
        case SEEA:
            if (!printAllStudent(headStu))
            {
                puts("Something Error!");
            }
            break;
        case SORT:
            if (!sortStudent(headStu))
            {
                puts("Something Error!");
            }
            else
            {
                puts("Sort OK!");
            }
            break;
        case GG:
            puts("Bye!");
            deleteAll(headStu);
            return 0;
        }
    }
}