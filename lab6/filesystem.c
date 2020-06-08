#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <unistd.h>
#include "filesystem.h"

void getDateAndTime(unsigned short *date, unsigned short *Time)
{
    time_t tempTime = time(NULL);
    struct tm *pTime = localtime(&tempTime);
    *date = ((((pTime->tm_year - 80) & 0b1111111) << 9) | \
        (((pTime->tm_mon) & 0b1111) << 5) | \
        ((pTime->tm_mday) & 0b11111));
    *Time = (((pTime->tm_hour & 0b11111) << 11) | \
        (((pTime->tm_min) & 0b111111) << 4) | \
        ((pTime->tm_sec) & 0b1111));
}

int findFreeBlock()
{
    pFat start = (pFat)&(myvhard[1 * BLOCKSIZE]);
    for(int i = 6; i < 1000; i++)
    {
        if(!(start + i)->id)
        {
            return i;
        }
    }
    return -1;
}

bool setFATTables(int index, unsigned short status)
{
    if(index > 1000)
    {
        return false;
    }
    pFat start = (pFat)&(myvhard[1 * BLOCKSIZE]);
    pFat start2 = (pFat)&(myvhard[3 * BLOCKSIZE]);
    (start + index)->id = status;
    (start2 + index)->id = status;
    return true;
}

void FCBToUSEROPEN(pFcb tempFCB, pUseropen tempUSEROPEN)
{
    memcpy(tempUSEROPEN->filename, tempFCB->filename, sizeof(tempFCB->filename));
    memcpy(tempUSEROPEN->exname, tempFCB->exname, sizeof(tempFCB->exname));
    tempUSEROPEN->attribute = tempFCB->attribute;
    tempUSEROPEN->time = tempFCB->time;
    tempUSEROPEN->date = tempFCB->date;
    tempUSEROPEN->length = tempFCB->length;
    tempUSEROPEN->fatIndex = tempFCB->fatIndex;
    tempUSEROPEN->count = 0;
    tempUSEROPEN->fcbstate = 0;
    tempUSEROPEN->topenfile = 1;    
}

void initDirFCB(pFcb dirFCB, char *name, unsigned short date, unsigned short time, int index)
{
    strcpy(dirFCB->filename, name);
    dirFCB->attribute = 0;
    dirFCB->date = date;
    dirFCB->time = time;
    dirFCB->length = 0;
    dirFCB->free = 1;
    dirFCB->fatIndex = index;
}

void initNormalFCB(pFcb fileFCB, char *name, char *exname, unsigned short date, unsigned short time, \
    unsigned long length, int index)
{
    strcpy(fileFCB->filename, name);
    if(exname) strcpy(fileFCB->exname, exname);
    fileFCB->attribute = 1;
    fileFCB->date = date;
    fileFCB->time = time;
    fileFCB->length = length;
    fileFCB->free = 1;
    fileFCB->fatIndex = index;
}

// pFcb findPurposeFCB(char *purpose, char *nowPath, pFcb dirFCB)
// {
//     pFcb temp = dirFCB;
//     pFcb ret = NULL;
//     for(int i = 0; ; i++)
//     {
//         char fullName[255] = {0, };
//         if(i && (temp == dirFCB))
//         {
//             return NULL;
//         }
//         strcpy(fullName, nowPath);
//         strcat(fullName, dirFCB->filename);
//         strcat(fullName, dirFCB->exname);
//         if(!strcmp(fullName, purpose))
//         {
//             return temp;
//         }
//         if(!temp->attribute)
//         {
//             ret = findPurposeFCB(purpose, fullName, temp);
//             if(ret) return ret;
//         }
//         temp = ;
//     }
// }

void my_format()
{
    unsigned short date, Time;
    memset(&myvhard[1 * BLOCKSIZE], FREE, BLOCKSIZE * 2);
    memset(&myvhard[3 * BLOCKSIZE], FREE, BLOCKSIZE * 2);
    memset(&myvhard[5 * BLOCKSIZE], FREE, BLOCKSIZE);
    getDateAndTime(&date, &Time);
    pFcb rootfcb = (pFcb)&myvhard[5 * BLOCKSIZE];
    initDirFCB(rootfcb, "/", date, Time, 5);
    initDirFCB(rootfcb + 1, ".", date, Time, 5);
    initDirFCB(rootfcb + 2, "..", date, Time, 5);
    rootfcb->length = 3 * sizeof(fcb);
    (rootfcb + 1)->length = 3 * sizeof(fcb);
    setFATTables(5, END);
}

void startsys()
{
    myvhard = (char*)malloc(sizeof(char) * SIZE);
    FILE *fp = fopen("temp.dmp", "rb");
    if(fp)
    {
        fread(myvhard, SIZE, 1, fp);
        fclose(fp);
    }
    else
    {
        my_format();
        curdir = 0;
        startp = &myvhard[5 * BLOCKSIZE];
    }
    strcpy(currentdir, "/");
    memset(openfilelist, 0, sizeof(useropen) * 10);
    FCBToUSEROPEN((pFcb)&myvhard[5 * BLOCKSIZE], &openfilelist[0]);
}

pFcb walkDirByFATIndex(int fatIndex, char *filename, char *exname)
{
    char finalName[12];
    for(int index = fatIndex; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for(int i = 0; i < FCBNUM; i++)
        {
            pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if(!tempFcb->free) continue;
            if(!strcmp(tempFcb->filename, filename))
            {
                if(exname)
                {
                    if(strcmp(tempFcb->exname, exname)) continue;
                }
                return tempFcb;
            }
        }
    }
    return NULL;
}

void parseLocation(char **parseList, char *location)
{
    int i = 0;
    char *tempLoc = location;
    while((tempLoc = strchr(location, '/')))
    {
        int length = (tempLoc-location)?(tempLoc-location):1;
        char *temp = (char*)malloc(sizeof(char) * (length + 1));
        memcpy(temp, location, length);
        temp[length] = 0;
        parseList[i++] = temp;
        while(*(location = ++tempLoc) == '/');
    }
    if(*location)
    {
        parseList[i] = strdup(location);
    }
}

void getAbsAddr(char *addr, char *ret, bool isDir)
{
    char *oriTemp[MAXDEPTH] = {0, };
    char *purposeTemp[MAXDEPTH] = {0, };
    char *finalTemp[MAXDEPTH] = {0, };
    int oriLength;
    parseLocation(oriTemp, currentdir);
    parseLocation(purposeTemp, addr);
    for(oriLength = 0; oriTemp[oriLength]; oriLength++);
    memcpy(finalTemp, oriTemp, sizeof(finalTemp));
    for(int i = 0; purposeTemp[i]; i++)
    {
        if(!strcmp(purposeTemp[i], "."))
        {
            continue;
        }
        else if(!strcmp(purposeTemp[i], ".."))
        {
            if(oriLength == 1) continue;
            finalTemp[--oriLength] = NULL;
        }
        else if(!strcmp(purposeTemp[i], "/")) continue;
        else
        {
            finalTemp[oriLength++] = purposeTemp[i];
        }
    }
    memset(ret, 0, 80);
    for(int i = 0; finalTemp[i]; i++)
    {
        strcat(ret, finalTemp[i]);
        if(i) strcat(ret, "/");
    }
    if(!isDir)
    {
        ret[strlen(ret) - 1] = 0;
    }
    for(int i = 0; oriTemp[i]; free(oriTemp[i++]));
    for(int i = 0; purposeTemp[i]; free(purposeTemp[i++]));
}

void splitAddr(char *addr, char *dir, char *filename, char *exname)
{
    char path[80] = {0, };
    if(dir)
    {
        getAbsAddr(addr, path, 0);
        char *final = strrchr(path, '/');
        memcpy(dir, path, final - path + 1);
        dir[final - path + 1] = 0;
    }
    char *final = strrchr(addr, '/');
    if(!final)
    {
        final = addr;
    }
    else if (final == addr);
    else
    {
        final = final + 1;
    }
    char *ex = NULL;
    int length = strlen(final);
    if(exname && length > 2)
    {
        if(ex = strchr(final, '.'))
        {
            strcpy(exname, ex + 1);
            length -= (ex - final + 1);
        }
    }
    memset(filename, 0, 8);
    memcpy(filename, final, length);
}

pFcb findPurposeFCB(char *purposeName, pFcb *purposeDirFCB)
{
    char *tempLoc[MAXDEPTH] = {0, };
    parseLocation(tempLoc, purposeName);
    int fatIndex, length;
    pFcb retNum, dirFcb;
    char name[8] = {0, };
    char exname[3] = {0, };
    if(!strcmp(tempLoc[0], "/"))
    {
        fatIndex = 5;
    }
    else
    {
        fatIndex = openfilelist[curdir].fatIndex;
    }
    dirFcb = (pFcb)&myvhard[fatIndex * BLOCKSIZE];
    for(int i = 0; tempLoc[i]; i++)
    {
        if(!tempLoc[i + 1])
        {
            if(purposeDirFCB)
            {
                *purposeDirFCB = dirFcb;
            }
            splitAddr(tempLoc[i], NULL, name, exname);
            retNum = walkDirByFATIndex(fatIndex, name, exname);
        }
        else
        {
            dirFcb = walkDirByFATIndex(fatIndex, tempLoc[i], NULL);
            if(!dirFcb || dirFcb->attribute)
            {
                retNum = NULL;
                break;
            }
        }
        fatIndex = dirFcb->fatIndex;
    }
    for(int i = 0; tempLoc[i]; free(tempLoc[i++]));
    return retNum;
}

pFcb extraFCB(pFcb purposeDirFCB)
{
    int fatIndex = purposeDirFCB->fatIndex;
    int blockIndex;
    for( ; ((pFat)(&myvhard[1 * BLOCKSIZE]) + fatIndex)->id != END; fatIndex = ((pFat)(&myvhard[1 * BLOCKSIZE]) + fatIndex)->id);
    if((blockIndex = findFreeBlock()) == -1)
    {
        printf("The disk is full!!!!!!\n");
        return NULL;
    }
    setFATTables(blockIndex, END);
    setFATTables(fatIndex, blockIndex);
    return (pFcb)&myvhard[blockIndex * BLOCKSIZE];
}

pFcb findFreeFCB(pFcb purposeDirFCB)
{
    int fatIndex = purposeDirFCB->fatIndex;
    for(int index = fatIndex; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for(int i = 0; i < FCBNUM; i++)
        {
            pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if(!tempFcb->free) return tempFcb;
        }
    }
    return extraFCB(purposeDirFCB);
}

int getFreeOpen(void)
{
    for(int i = 0; i < MAXOPENFILE; i++)
    {
        if(!openfilelist[i].topenfile) return i;
    }
    for(int i = 0; i < MAXOPENFILE; i++)
    {
        if(i == curdir) continue;
        return i;
    }
}

void my_cd(char *dirname)
{
    pFcb temp = findPurposeFCB(dirname, NULL);
    if(!temp)
    {
        printf("%s: No such file or dictonary!\n", dirname);
        return;
    }
    if(temp)
    {
        if(temp->attribute)
        {
            printf("%s: Not such dictonary!\n", dirname);
            return;
        }
        int tempNum = curdir;
        curdir = getFreeOpen();
        memset(&openfilelist[tempNum], 0, sizeof(useropen));
        FCBToUSEROPEN(temp, &openfilelist[curdir]);
        getAbsAddr(dirname, currentdir, true);
    }
}

void my_mkdir(char *dirname)
{
    char *temp[MAXDEPTH] = {0, };
    int length;
    pFcb dirFcb = NULL;
    unsigned short date, time;
    int index;
    parseLocation(temp, dirname);
    if(findPurposeFCB(dirname, &dirFcb))
    {
        printf("%s: It's already exist!\n", dirname);
        return;
    }
    for(length = 0; temp[length++];);
    length--;
    if(!dirFcb)
    {
        printf("%s: No such file or dictonary\n", dirname);
        return;
    }
    if((index = findFreeBlock()) == -1) return;
    memset(&myvhard[BLOCKSIZE * index], 0, BLOCKSIZE);
    setFATTables(index, END);
    getDateAndTime(&date, &time);
    initDirFCB((pFcb)(&myvhard[BLOCKSIZE * index]), ".", date, time, index);
    initDirFCB((pFcb)(&myvhard[BLOCKSIZE * index]) + 1, "..", date, time, dirFcb->fatIndex);
    ((pFcb)&(myvhard[BLOCKSIZE * index]))->length = 2 * sizeof(fcb);
    pFcb newDirFCB = findFreeFCB(dirFcb);
    initDirFCB(newDirFCB, temp[--length], date, time, index);
    newDirFCB->length = 2 * sizeof(fcb);
    dirFcb->length += sizeof(fcb);
    if(dirFcb->fatIndex == 5) (dirFcb + 1)->length += sizeof(fcb);
    for(int i = 0; temp[i]; free(temp[i++]));
}

void my_rmdir(char *dirname)
{
    pFcb purposeDir, parentDir;
    int index;
    if(purposeDir = findPurposeFCB(dirname, &parentDir))
    {
        if(!purposeDir->attribute)
        {
            for(int index = purposeDir->fatIndex; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
            {
                for(int i = 0; i < FCBNUM; i++)
                {
                    pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
                    if(!strcmp(tempFcb->filename, "..") || !strcmp(tempFcb->filename, ".")) continue;
                    if(tempFcb->free)
                    {
                        printf("rmdir failed cause this dir is not empty\n");
                        return;
                    }
                }
            }
            for(index = purposeDir->fatIndex; ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
            {
                setFATTables(index, 0);
            }
            setFATTables(index, 0);
            purposeDir->free = 0;
            parentDir->length -= sizeof(fcb);
            if(parentDir->fatIndex == 5) (parentDir + 1)->length -= sizeof(fcb);
        }
        else
        {
            printf("rmdir failed: %s is not a dictonary\n", dirname);
            return;
        }
    }
    else
    {
        printf("No such file or dictonary\n");
    }
}

void my_ls(void)
{
    int index = openfilelist[curdir].fatIndex;
    for( ; index != END; index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id)
    {
        for(int i = (index==5?1:0); i < FCBNUM; i++)
        {
            pFcb tempFcb = ((pFcb)&myvhard[index * BLOCKSIZE]) + i;
            if(!tempFcb->free) continue;
            if(tempFcb->exname[0] != '\x00')
            {
                printf("%s.%s\t%d.%d.%d:%d:%d:%d\t%d\t%s\n", \
                    tempFcb->filename, \
                    tempFcb->exname, \
                    ((tempFcb->date >> 9) & 0b1111111) + 1980, \
                    (tempFcb->date >> 5) & 0b1111, \
                    (tempFcb->date & 0b11111),
                    (tempFcb->time >> 11) & 0b11111,
                    (tempFcb->time >> 4) & 0b111111,
                    tempFcb->time & 0b1111,
                    tempFcb->length,
                    typeStr[tempFcb->attribute]);
            }
            else
            {
                printf("%s\t%d.%d.%d:%d:%d:%d\t%d\t%s\n", \
                    tempFcb->filename, \
                    ((tempFcb->date >> 9) & 0b1111111) + 1980, \
                    (tempFcb->date >> 5) & 0b1111, \
                    (tempFcb->date & 0b11111),
                    (tempFcb->time >> 11) & 0b11111,
                    (tempFcb->time >> 4) & 0b111111,
                    tempFcb->time & 0b1111,
                    tempFcb->length,
                    typeStr[tempFcb->attribute]);
            }
        }
    }
}

void my_create(char *filename)
{
    pFcb purposeDir = NULL, parentDir = NULL;
    unsigned short date, time;
    int length;
    char tempName[8];
    char exname[3];
    if(purposeDir = findPurposeFCB(filename, &parentDir))
    {
        printf("%s: File exist!\n", filename);
        return;
    }
    else
    {
        if(!parentDir)
        {
            printf("%s: No such file or dictonary!\n", filename);
            return;
        }
        int fatIndex = findFreeBlock();
        setFATTables(fatIndex, END);
        pFcb fileFCB = findFreeFCB(parentDir);
        getDateAndTime(&date, &time);
        splitAddr(filename, NULL, tempName, exname);
        initNormalFCB(fileFCB, tempName, exname, date, time, 0, fatIndex);
        parentDir->length += sizeof(fcb);
        if(parentDir->fatIndex == 5) (parentDir + 1)->length += sizeof(fcb);
    }
}

void my_rm(char *filename)
{
    pFcb purposeDir = NULL, parentDir = NULL;
    int index;
    if(!(purposeDir = findPurposeFCB(filename, &parentDir)))
    {
        printf("%s: No such file or dictnary!\n", filename);
        return;
    }
    else
    {
        if(!purposeDir->attribute)
        {
            printf("%s: It's not a file!\n", filename);
            return;
        }        
        else
        {
            for(int i = 0; i < MAXOPENFILE; i++)
            {
                if(!openfilelist[i].topenfile || !openfilelist[i].attribute) continue;
                if(!strcmp(purposeDir->filename, openfilelist[i].filename) && !strcmp(purposeDir->exname, openfilelist[i].exname))
                {
                    printf("File is opened! Please close it first!\n");
                    return;
                }
            }
            for(index = purposeDir->fatIndex; ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id != END;)
            {
                int temp = index;
                index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id;
                setFATTables(temp, 0);
            }
            setFATTables(index, 0);
            purposeDir->free = 0;
            parentDir->length -= sizeof(fcb);
            if(parentDir->fatIndex == 5) (parentDir + 1)->length -= sizeof(fcb);
        }
    }
}

void my_open(char *filename)
{
    char path[80] = {0, };
    getAbsAddr(filename, path, false);
    char dir[80] = {0, }, name[8] = {0, }, exname[3] = {0, };
    pFcb purposeFCB;
    splitAddr(path, dir, name, exname);
    for(int i = 0; i < MAXOPENFILE; i++)
    {
        if(!openfilelist[i].topenfile) continue;
        if(!strcmp(dir, openfilelist[i].dir) && !strcmp(name, openfilelist[i].filename) && \
            !strcmp(exname, openfilelist[i].exname))
        {
            printf("File had opened!\n");
            return;
        }
    }
    purposeFCB = findPurposeFCB(filename, NULL);
    if(!purposeFCB)
    {
        printf("No such file!\n");
        return;
    }
    int index = getFreeOpen();
    printf("Now get fd:%d\n", index);
    FCBToUSEROPEN(purposeFCB, &openfilelist[index]);
    strcpy(openfilelist[index].dir, dir);
}

void my_close(int fd)
{
    pFcb dirFCB, fileFCB;
    char path[80] = {0, };
    if(!openfilelist[fd].topenfile)
    {
        printf("This fd is invalid!\n");
        return;
    }
    strcpy(path, openfilelist[fd].dir);
    strcat(path, openfilelist[fd].filename);
    if(openfilelist[fd].exname[0])
    {
        strcat(path, ".");
        strcat(path, openfilelist[fd].exname);
    }
    fileFCB = findPurposeFCB(path, &dirFCB);
    if(!fileFCB)
    {
        printf("Something error!!\n");
        return;
    }
    if(openfilelist[fd].fcbstate) fileFCB->length = openfilelist[fd].length;
    memset(&openfilelist[fd], 0, sizeof(useropen));
}

int do_write(int fd, char *text, int len, char wstyle)
{
    int ret = len, temp;
    int index = openfilelist[fd].fatIndex;
    char buf[BLOCKSIZE] = {0, };
    switch (wstyle)
    {
    case '+':
        openfilelist[fd].count = openfilelist[fd].length;
        break;
    case 'w':
        openfilelist[fd].count = 0;
        break;
    case 'a':
        break;
    default:
        printf("No such mode!\n");
        return -1;
    }
    for(int i = 0; i < openfilelist[fd].count / BLOCKSIZE; i++)
    {
        temp = index;
        index = ((pFat)&(myvhard[BLOCKSIZE * 1]) + index)->id;
    }
    while(len > 0)
    {
        if(index == END)
        {
            int tempIndex = findFreeBlock();
            setFATTables(temp, tempIndex);
            setFATTables(tempIndex, END);
            index = tempIndex;
        }
        int offset = openfilelist[fd].count % BLOCKSIZE;
        if(offset) memcpy(buf, &myvhard[BLOCKSIZE * index], BLOCKSIZE);
        int writeLength = len<BLOCKSIZE?len:BLOCKSIZE-offset;
        memcpy(buf + offset, text, writeLength);
        memcpy(&myvhard[BLOCKSIZE * index], buf, writeLength);
        openfilelist[fd].count += writeLength;
        len -= writeLength;
        temp = index;
        index = ((pFat)(&myvhard[BLOCKSIZE * 1]) + index)->id;
    }
    index = openfilelist[fd].fatIndex;
    openfilelist[fd].fcbstate = 1;
    if(wstyle == 'w' && openfilelist[fd].count < openfilelist[fd].length)
    {
        int orgNum = openfilelist[fd].length / BLOCKSIZE;
        int purNum = openfilelist[fd].count / BLOCKSIZE;
        for(int i = 0; i < purNum; i++) index = ((pFat)(&myvhard[BLOCKSIZE * 1]) + index)->id;
        temp = ((pFat)(&myvhard[BLOCKSIZE * 1]) + index)->id;
        setFATTables(index, END);
        index = temp;
        for(int i = 0, temp = 0; i < orgNum - purNum; i++)
        {
            temp = index;
            index = ((pFat)(&myvhard[BLOCKSIZE * 1]) + index)->id;
            setFATTables(temp, 0);
        }        
    }
    openfilelist[fd].length = openfilelist[fd].count;
    return ret;
}

int my_write(int fd)
{
    char *tempBuf = (char*)malloc(sizeof(char) * BLOCKSIZE), tempChar;
    int bufNum = 1, index = 0;
    if(!openfilelist[fd].topenfile || !openfilelist[fd].attribute)
    {
        printf("This fd is invalid!\n");
        return -1;
    }
    char choice;
    printf("Please choose one way to input(a, w or +)\n");
    scanf("%c", &choice);
    getchar();
    printf("Now Please input\n");
    while(read(STDIN_FILENO, &tempChar, 1) != 0)
    {
        tempBuf[index++] = tempChar;
        if(!(index % 1024) && index != 0) tempBuf = realloc(tempBuf, BLOCKSIZE * (++bufNum));
    }
    tempBuf[index] = 0;
    int ret = do_write(fd, tempBuf, index, choice);
    free(tempBuf);
    return ret;
}

int do_read(int fd, int len, char *text)
{
    // if(!text)
    // {
    //     printf("%s\n", funcList[9].describe);
    //     return;
    // }
    printf("%d\n", len);
    int maxLen = openfilelist[fd].length - openfilelist[fd].count;
    printf("%d, %d\n",openfilelist[fd].length, openfilelist[fd].count);
    int ret, length = ret = len>maxLen?maxLen:len;
    int logicOffset = openfilelist[fd].count % BLOCKSIZE;
    int logicIndex = openfilelist[fd].count / BLOCKSIZE;
    char *readBuf = (char*)malloc(sizeof(char) * BLOCKSIZE);
    int index = openfilelist[fd].fatIndex;
    for(int i = 0; i < logicIndex; i++) index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + i)->id;
    while(length > BLOCKSIZE)
    {
        memcpy(readBuf, &myvhard[BLOCKSIZE * index], BLOCKSIZE);
        memcpy(text, readBuf + logicOffset, BLOCKSIZE - logicOffset);
        length -= (BLOCKSIZE - logicOffset);
        logicOffset = 0;
        index = ((pFat)(&myvhard[1 * BLOCKSIZE]) + index)->id;
    }
    memcpy(readBuf, &myvhard[BLOCKSIZE * index], BLOCKSIZE);
    memcpy(text, readBuf, length);
    return ret;
}

int my_read(int fd, int len)
{
    char *text = (char*)malloc(sizeof(char) * len);
    if(!openfilelist[fd].topenfile || !openfilelist[fd].attribute)
    {
        printf("This fd is invalid!\n");
        return -1;
    }
    int ret = do_read(fd, len, text);
    printf("%d\n", ret);
    for(int i = 0; i < ret; i++) write(STDOUT_FILENO, &text[i], 1);
    return ret;
}

void my_print()
{
    for (int i = 0; i < 2 * BLOCKSIZE / sizeof(fat); i++)
    {
        if((i % 10 == 0) && i) printf("\n");
        printf("%hd\t", ((pFat)(&myvhard[1 * BLOCKSIZE]) + i)->id);
    }
    printf("\n");
}

void my_exitsys()
{
    FILE *fp = fopen("temp.dmp", "wb");
    fwrite(myvhard, SIZE, 1, fp);
    fclose(fp);
    free(myvhard);
    printf("Exit~\n");
    exit(-1);
}

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
            arg[i - 2] = strndup(buf, temp - buf);
        }
        buf = temp + 1;
    }
    i==1?(strcpy(cmd, buf)):(arg[i - 2] = strdup(buf));
}

void my_help(void)
{
    for(int i = 0; i < (sizeof(funcList) / sizeof(func)); i++) printf("%s", funcList[i].describe);
}

bool isNum(char *string)
{
    for(int i = 0; string[i]; i++)
    {
        if('0' > string[i] || string[i] > '9')
        {
            return false;
        }
    }
    return true;
}

int main()
{
    startsys();
    printf("Welcome to beta filesystem!\n");
    while(1)
    {
        char order[80] = {0, };
        char cmd[0x10] = {0, };
        char *arg[0x10] = {NULL, };
        int arg1, arg2;
        bool flag = false;
        printf("%s> ", currentdir);
        fgets(order, 80, stdin);
        order[strlen(order) - 1] = '\x00';
        exprBuf(order, cmd, arg);
        for(int i = 0; i < (sizeof(funcList) / sizeof(func)); i++)
        {
            if(!strcmp(funcList[i].name, cmd))
            {
                switch (funcList[i].type)
                {
                case 0:
                    funcList[i].func();
                    flag = true;
                    break;
                case 1:
                    funcList[i].func(arg[0]);
                    flag = true;
                    break;
                case 2:
                    if(!isNum(arg[0]))
                    {
                        printf("Args is invalid!\n");
                        break;
                    }
                    arg1 = atoi(arg[0]);
                    funcList[i].func(arg1);
                    flag = true;
                    break;
                case 3:
                    if(!isNum(arg[0]) || !isNum(arg[1]))
                    {
                        printf("Args is invalid!\n");
                        break;
                    }
                    arg1 = atoi(arg[0]);
                    arg2 = atoi(arg[1]);
                    funcList[i].func(arg1, arg2);
                    flag = true;
                    break;
                default:
                    break;
                }
            }
            if(flag) break;
            if(i == (sizeof(funcList) / sizeof(func)) - 1)
            {
                printf("No such order!\n");
            }
        }
    }
    return 0;
}