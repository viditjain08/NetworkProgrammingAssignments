#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<stdbool.h>
#include<signal.h>
#include <pthread.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <fcntl.h>
#include <sys/mman.h>


#define MAX_SIZE 100
#define NR_JOBS 20
//char **lookuptable;
typedef struct {
    long mesg_type;
    char mesg_text[MAX_SIZE];
} message;

struct job {
    int id;
    char *command;
    pid_t pid;
    pid_t pgid;
    int mode;
};
typedef struct job job;
struct shell_info {
    struct job *jobs[NR_JOBS + 1];
};
struct shell_info *shell;
void handler(int signo);
bool check (char *filepath);
void createEmptyFile(char *filename);
void replaceInFile(char *filename, int index, char **argv, int nofargs);
char *indexInFile(FILE *fp, int index);
char *getfile(char *path,char *firstarg);
char **getargv(char *buff,int *nofargs,bool *back_pr);
char *getcommand(char *buf,int *noofargs);
void parsecommand(char *completepath,char *p,char **argv,int nofargs);
int strinarr(char **argv, char *sym, int nofargs,int start_index);
char **subarray(char **argv, int start, int end);
void lessersign(char *p,char **argv,int nofargs);
void greatersign(char *p,char **argv,int nofargs);
void pipesign(char *completepath,char *path,char **argv,int nofargs);
void pipecommand(char *completepath,char *path,char **argv,int nofargs);
void lessersign(char *p,char **argv,int nofargs);
char *nextToken(char *str,int *point);
void doubleMessageQueue(char *path1,char **argv,int nofargs);
void parsecommand2(char *completepath,char *p,char **argv,int nofargs);
void customcommands(char **argv,int nofargs);
void deleteLineFromFile(char *filename, int index);
void insertLineIntoFile(char *filename, int index, char *text);
char *getLineFromFile(char *filename, int index);
void shmPipe(char *completepath,char *path,char **argv,int nofargs);
void messageQueuePipe(char *completepath,char *path,char **argv,int nofargs);
void doubleMessageQueue(char *path1,char **argv,int nofargs);
void doubleshm(char *path1,char **argv,int nofargs);
int insert_job(job *j);
int get_next_job_id();
struct my_msgbuf
{
    long mtype;
    char mtext[1000];
};
