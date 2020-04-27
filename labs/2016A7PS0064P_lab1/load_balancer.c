#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>

FILE *fptr;
int *busy;
int *pids;


void handle_sig1(int sig, siginfo_t *si, void *ucontext)
{
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while(!feof(fptr) && (read = getline(&line, &len, fptr)) == -1);
    fflush(fptr);
    sleep(1);
    if(feof(fptr)) {
        printf("Process %d terminated\n",si->si_value.sival_int);
        exit(0);
    }
    printf("Child Process %d:\t\t\t\t\t\t\t%s\n",getpid(),line);
    union sigval sv;
    sv.sival_int = si->si_value.sival_int;
    sigqueue(getppid(), SIGUSR2, sv);

}

void handle_sigchild(int sig) {
    fclose(fptr);
    kill(0,SIGKILL);
    exit(0);
}
void handler_sig2(int sig, siginfo_t *si, void *ucontext)
{
    printf("Process %d available\n",si->si_value.sival_int); /* This is what you're looking for. */
    busy[si->si_value.sival_int]=0;
}


int main() {

    signal(SIGCHLD, handle_sigchild);
    int n;
    printf("Enter number of child processes\n");
    scanf("%d",&n);
    char filename[1024];
    printf("Enter path of file to be read\n");
    scanf("%s",filename);

    struct sigaction sa1;
    sigemptyset(&sa1.sa_mask);
    sa1.sa_sigaction = handle_sig1;
    sa1.sa_flags = SA_SIGINFO; /* Important. */

    sigaction(SIGUSR1, &sa1, NULL);

    struct sigaction sa2;
    sigemptyset(&sa2.sa_mask);
    sa2.sa_sigaction = handler_sig2;
    sa2.sa_flags = SA_SIGINFO; /* Important. */

    sigaction(SIGUSR2, &sa2, NULL);


    char str[1000];
    char * line = NULL;
    size_t len = 0;
    ssize_t read;


    fptr = fopen(filename, "r");

    if (fptr == NULL)
    {
        printf("Cannot open file \n");
        exit(0);
    }
    setvbuf(fptr, NULL, _IOLBF, 0);
    setvbuf(stdout, NULL, _IOLBF, 0);

    pid_t pid=-1;
    int flag=0;
    busy = (int*)malloc(n*sizeof(int));
    pids = (int*)malloc(n*sizeof(int));

    for(int i=0;i<n;i++) {
        if(pid!=0) {
            pid=fork();
            pids[i]=pid;
        }
        busy[i]=0;
    }

    if(pid!=0) {
        // Parent Process
        printf("Parent Process ID: %d\n",getpid());
        sleep(1);

        int ptr=0;
        while(flag==0) {
            if(busy[ptr]==0) {
                printf("Process %d starting read\n",ptr);
                busy[ptr]=1;

                union sigval sv;
                sv.sival_int = ptr;
                sigqueue(pids[ptr], SIGUSR1, sv);
                sleep(0.01);
            }
            ptr=(ptr+1)%n;
        }
    } else {
        // Child Processes
        printf("Child Process ID: %d\n",getpid());
        while(1) {
            pause();
        }
    }



    return 0;
}
