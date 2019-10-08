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
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define MAX_SIZE 1024
extern char **environ;
#define PORT 8080
#define SA struct sockaddr


void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}

char* getip() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*)
                           host_entry->h_addr_list[0]));

    return IPbuffer;
}

typedef struct {
    char name[10];
    char ip[20];
    int active;
} node;

int n;
int pipefds[2];
int* sockfds;
node *list;
int idx=0;

void createChildProcess() {
    fflush (stdout);
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf ("prompt> ");
    printf("%s$ ",cwd);
    fflush (stdout);
    char buf[MAX_SIZE];
    int length = read(0,buf,MAX_SIZE);
    buf[length]='\0';
    for (int i = 0; i < MAX_SIZE; i++)
        if(buf[i] == '\n') {
            buf[i]='\0';
        }
    write(pipefds[1], buf, sizeof(buf));
    int ret = kill(getppid(),SIGUSR1);
    exit(0);
}

void initializesockets() {

    sockfds = (int*)malloc(sizeof(int)*n);
    struct sockaddr_in servaddr, cli;

    // socket create and varification
    for(int i=0;i<n;i++) {
        sockfds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfds[i] == -1) {
            printf("socket creation failed...\n");
            exit(0);
        }
    }
    // sleep(10);

    while(1) {
        // printf("vidit\n");
        // sleep(1);
        for(int i=0;i<n;i++) {
            if(list[i].active==1) continue;
            bzero(&servaddr, sizeof(servaddr));

            // assign IP, PORT
            servaddr.sin_family = AF_INET;
            servaddr.sin_addr.s_addr = inet_addr(list[i].ip);
            servaddr.sin_port = htons(PORT);
            // connect the client socket to server socket
            if (connect(sockfds[i], (SA*)&servaddr, sizeof(servaddr)) == 0) {
                list[i].active = 1;
                char *home = getenv("HOME");
                send(sockfds[i] , home , strlen(home) , 0 );
                // printf("Home sent to server\n");
            }
        }
    }

}

int checkpipe(char* msg) {
    char* temp1 = strsep(&msg,"|");
    char* temp2 = strsep(&msg,"|");
    if(temp2==NULL) {
        return 0;
    } else {
        return 1;
    }
}

int findnode(char *message) {
    char *x = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(x,message);
    char *tmp = strsep(&x,".");
    if(strcmp(tmp,"n*")==0) {
        return -1;
    } else {
        for(int i=0;i<n;i++) {
            if(strcmp(tmp,list[i].name)==0) {
                return i;
            }
        }
    }
    return -2;
}

void runcommand(char* msg, int nodeidx, int ispipe) {
    if(nodeidx==-2) {
        int count=2;
        for(int i=0;msg[i]!='\0';i++) {
            if(msg[i]==' ') {
                count++;
            }
        }
        char** arg = (char**)malloc(sizeof(char*)*count);
        for(int i=0;i<count;i++) {
            arg[i] = (char*)malloc(sizeof(char)*MAX_SIZE);
        }
        int ptr;
        for (ptr = 0; ptr < count; ptr++) {
            arg[ptr] = strsep(&msg, " ");
            if (arg[ptr] == NULL) {
                break;
            }

            if (strlen(arg[ptr]) == 0)
                ptr--;

        }

        if(strcmp(arg[0],"cd")==0) {
            chdir(arg[1]);
            return;
        }
        int p = fork();
        if(p==0) {
            execvp(arg[0],arg);
            exit(0);
        }
        int stat_loc;
        waitpid(p, &stat_loc, WUNTRACED);
        sleep(0.5);
        // system(msg);
    } else if(nodeidx!=-1) {
        if(list[nodeidx].active==0) {
            printf("Node not active, cannot run command\n");
            return;
        }
        int p = fork();
        if(p==0) {
            char *temppipe = (char*)malloc(sizeof(char)*2);
            if(ispipe==0) {
                strcpy(temppipe,"0");
            } else {
                strcpy(temppipe,"1");
            }
            send(sockfds[nodeidx],temppipe,1+strlen(temppipe),0);
            send(sockfds[nodeidx], msg , 1+strlen(msg) , 0);
            char *out = (char*)malloc(sizeof(char)*MAX_SIZE);
            int val = read(sockfds[nodeidx] , out, MAX_SIZE);
            if(val!=-1) {
                out[val]='\0';
                printf("%s",out);
            }
        }
        int stat_loc;
        waitpid(p, &stat_loc, WUNTRACED);

        close(sockfds[nodeidx]);
        sockfds[nodeidx] = socket(AF_INET, SOCK_STREAM, 0);

        struct sockaddr_in servaddr, cli;
        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(list[nodeidx].ip);
        servaddr.sin_port = htons(PORT);
        // connect the client socket to server socket
        if (connect(sockfds[nodeidx], (SA*)&servaddr, sizeof(servaddr)) == 0) {
            // printf("Home sent to server\n");
        } else {
            list[nodeidx].active=0;
        }

    } else {
        for(int i=0;i<n;i++) {
            if(list[i].active==1) {
                char *temp = (char*)malloc(sizeof(char)*MAX_SIZE);
                strcpy(temp,msg);
                if(i==idx) {
                    runcommand(temp, -2, ispipe);
                } else {
                    runcommand(temp, i, ispipe);
                }
            }
        }
    }

    // char** arg = (char**)malloc(sizeof(char*)*2);
    // arg[2] = NULL;
    // arg[0] = (char*)malloc(sizeof(char)*5);
    // arg[1] = (char*)malloc(sizeof(char)*10);
    // strcpy(arg[0],"cd");
    // strcpy(arg[1],"../..");
    // execvp(arg[0],arg);
    // arg[1]=NULL;
    // chdir("../..");
    // strcpy(arg[0],"ls");
    // execvp(arg[0],arg);
}
void executeshell(char* msg) {
    if(strcmp(msg,"nodes")==0) {
        runcommand("sleep 0.00001", -1, 0);
        printf("Current Active Nodes\n");
        for(int i=0;i<n;i++) {
            if(list[i].active==1) {
                printf("%s--->%s\n",list[i].name,list[i].ip);
            }
        }
        return;
    }
    char *x = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(x,msg);
    int ispipe = checkpipe(x);
    // printf("ISPIPE %d\n",ispipe);
    if(ispipe==0) {
        int nodeidx = findnode(msg);

        // if(nodeidx==-1) {
        //     printf("Command for all nodes\n");
        // } else if(nodeidx==-2) {
        //     printf("Local command\n");
        // } else {
        //     printf("Command for %s\n",list[nodeidx].name);
        // }
        if(nodeidx!=-2) {
            strcpy(x,msg);
            x = strsep(&x,".");
            msg+=(strlen(x)+1);
            if(nodeidx==idx) {
                nodeidx=-2;
            }
        }
        // printf("%s\n",msg);
        runcommand(msg,nodeidx,0);

    } else {
        strcpy(x,msg);
        pid_t wpid;
        int status = 0;
        char *y = (char*)malloc(sizeof(char)*MAX_SIZE);
        int count=0;
        int c1=0;
        for(int i=0;msg[i]!='\0';i++) {
            if(msg[i]=='|') c1++;
        }
        int fd[2];
        pipe(fd);
        while( (y = strsep(&x,"|")) != NULL ) {
            int nodeidx = findnode(y);
            if(nodeidx!=-2) {
                char *z = (char*)malloc(sizeof(char)*MAX_SIZE);
                strcpy(z,y);
                z = strsep(&z,".");
                y+=(strlen(z)+1);
                if(nodeidx==idx) {
                    nodeidx=-2;
                }
            }
            if(fork()==0) {
                exit(0);
            }
            count++;
            wait(NULL);
        }
        // for(int i=0; i<c1; i++){
        //   wait(NULL);
        // }
        printf("jsbajbabvj\n");
    }
}

void handle_child_term(int sig) {
    wait(NULL);
    char readmessage[MAX_SIZE];
    read(pipefds[0], readmessage, sizeof(readmessage));
    executeshell(readmessage);
    if(fork()==0) {
        createChildProcess();
    }
}


int main() {

    // char buf[MAX_SIZE];
    // int length = read(0,buf,MAX_SIZE);
    // buf[length]='\0';
    // // printf("HOME : %s\n", getenv("HOME"));
	// system(buf);
    FILE *fp;
    char buff[255];
    signal(SIGUSR1, handle_child_term);

    if(pipe(pipefds)==-1) {
        printf("Pipe Creation failed...\n");
        exit(0);
    }

    fp = fopen("./config_file", "r");
    fscanf(fp, "%d\n", &n);
    printf("Number of Nodes: %d\n",n);
    list = (node*)malloc(sizeof(node)*n);
    for(int i=0;i<n;i++) {
        char temp1[255];
        list[i].active = 0;
        fgets(temp1, sizeof(temp1), fp);
        int ptr=0;
        while(temp1[ptr]!=' ') {
            list[i].name[ptr] = temp1[ptr];
            ptr++;
        }
        list[i].name[ptr]='\0';
        ptr++;
        int j=0;
        while(temp1[ptr]!='\0' && temp1[ptr]!='\n') {
            list[i].ip[j] = temp1[ptr];
            j++;
            ptr++;
        }
        list[i].ip[j]='\0';
        printf("%s------>%s\n",list[i].name,list[i].ip);

    }
    fclose(fp);
    for(int i=0;i<n;i++) {
        if(strcpy(getip(),list[i].ip)==0) {
            idx=i;
            break;
        }
    }
    list[idx].active = 1;
    printf("This is N%d\n",idx+1);

    if(fork()>0) {
        initializesockets();
    } else {
        createChildProcess();
    }


    // sleep(5);
    // printf("Enter Socket ID\n");
    // int sock;
    // scanf("%d",&sock);
    // char *hello = "Hello from client";
    // send(sock , hello , strlen(hello) , 0 );
    // printf("Hello message sent\n");




    return 0;
}
