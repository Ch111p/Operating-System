#define BLOCKSIZE 1024
#define SIZE 1024000
#define END 65535
#define FREE 0
#define ROOTBLOCKNUM 2
#define MAXOPENFILE 10
#define FCBNUM 32
#define MAXDEPTH 32

#pragma pack (8)
typedef struct FCB
{
    char filename[8];
    char exname[3];
    unsigned char attribute; //0 dir 1 file
    unsigned short time;
    unsigned short date;
    unsigned short fatIndex;
    unsigned int length;
    int useless;
    char free; //0 free 1 located
}fcb, *pFcb;
#pragma pack()

typedef struct FAT
{
    unsigned short id;
}fat, *pFat;

typedef struct USEROPEN
{
    char filename[8];
    char exname[3];
    unsigned char attribute;
    unsigned short time;
    unsigned short date;
    unsigned short fatIndex;
    unsigned long length;
    char dir[80];
    int count;
    char fcbstate;
    char topenfile;
}useropen, *pUseropen;

typedef struct BLOCK0
{
    char information[200];
    unsigned short root;
    unsigned char *startblock;
}block0, *pBlock0;

typedef struct funcStruct
{
    char *name;
    void (*func)();
    int type;
    char *describe;
}func, *pFunc;

unsigned char *myvhard;
useropen openfilelist[MAXOPENFILE];
int curdir;
char currentdir[80];
unsigned char *startp;
char buf[BLOCKSIZE];
void my_cd(char *dirname);
void my_mkdir(char *dirname);
void my_rmdir(char *dirname);
void my_ls(void);
void my_create(char *filename);
void my_rm(char *filename);
void my_open(char *filename);
void my_close(int fd);
int my_write(int fd);
int my_read(int fd, int len);
void my_help(void);
void my_exitsys();
void my_print(void);
func funcList[] = 
{
    {"cd", my_cd, 1, "usage: cd directory\n"}, 
    {"mkdir", my_mkdir, 1, "usage: mkdir directory\n"},
    {"rmdir", my_rmdir, 1, "usage: rmdir directory\n"},
    {"ls", my_ls, 0, "usage: ls\n"},
    {"create", my_create, 1, "usage: create FILENAME\n"},
    {"rm", my_rm, 1, "usage: rm FILENAME\n"},
    {"open", my_open, 1, "usage: open FILENAME\n"},
    {"close", my_close, 2, "usage: close FD\n"},
    {"write", my_write, 2, "usage: write FD\n"},
    {"read", my_read, 3, "usage: read FD LEN\n"},
    {"help", my_help, 0, "usage: help\n"},
    {"exit", my_exitsys, 0, "usage: exit\n"},
    {"print", my_print, 0, "usage: print\n"}
};

char *typeStr[] = {"dir", "file"};