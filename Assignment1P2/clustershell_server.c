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
#include <unistd.h>
#include <fcntl.h>
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
} node;

int main() {
    char run_cwd[MAX_SIZE];
    getcwd(run_cwd, sizeof(run_cwd));
    FILE *fp;
    char *buff;
    buff = (char*)malloc(sizeof(char)*255);

    int n;
    fp = fopen("./config_file", "r");
    fscanf(fp, "%d\n", &n);
    printf("Number of Nodes: %d\n",n);
    node *list;
    list = (node*)malloc(sizeof(node)*n);
    for(int i=0;i<n;i++) {
        char temp1[255];
        memset(list[i].ip, '\0', 20*sizeof(char));
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
    int idx=0;
    for(int i=0;i<n;i++) {
        if(strcpy(getip(),list[i].ip)==0) {
            idx=i;
            break;
        }
    }
    // idx=1;     // To run on same machine, manually set the idx to 1 since the code doesn't allow one-to-many name-to-ip
    printf("This is node N%d\n",idx+1);



    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));
    int opt = 1;
    // Forcefully attaching socket to the port 8080
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                                                  &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);

    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("socket bind failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {
        printf("Listen failed...\n");
        exit(0);
    }
    else
        printf("Server listening..\n");
    len = sizeof(cli);

    // Accept the data packet from client and verification
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("server acccept failed...\n");
        exit(0);
    }
    else
        printf("server acccept the client...\n");
    int valread = read(connfd , buff, 255);
    // printf("%s\n",buff);
    chdir(buff);

    while(1) {
        char cwd[1024];
        char *msg;
        msg = (char*)malloc(sizeof(char)*MAX_SIZE);
        getcwd(cwd, sizeof(cwd));
        printf("Current Working Directory: %s$\n",cwd);
        char *temppipe = (char*)malloc(sizeof(char)*2);

        int pipeval = read(connfd,temppipe,2);
        if(pipeval==0) {
            printf("Client terminated\n---------------------------------------------------\n");
            // sleep(10);
            chdir(run_cwd);
            char *args[]={"bash","run.sh","clustershell_server",NULL};
            execvp(args[0],args);
        }
        // while(pipeval<=0) {
        //     printf("%d\n",pipeval);
        //     pipeval = read(connfd,temppipe,2);
        // }
        temppipe[pipeval]='\0';

        int ispipe;
        if(strcmp(temppipe,"0")==0) {
            ispipe=0;
        } else {
            ispipe=1;
        }

        int val = read(connfd , msg, MAX_SIZE);
        // printf("Bytes read: %d\n",val);
        msg[val]='\0';
        // printf("Msg received: %s\n",msg);
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
            // printf("Changed to new directory %s\n",arg[1]);
        } else {
            int execpipe[2];
            char *in = (char*)malloc(sizeof(char)*MAX_SIZE);
            if(ispipe==1) {
                int tempval = read(connfd, in, MAX_SIZE);
                in[tempval] = '\0';
                // printf("%s-%d\n",in,tempval);
                pipe(execpipe);
            }
            int p = fork();
            if(p==0) {
                if(ispipe==1) {
                    close(execpipe[1]);
                    dup2(execpipe[0],STDIN_FILENO);
                    close(execpipe[0]);
                }
                dup2(connfd,STDOUT_FILENO);
                execvp(arg[0],arg);
                exit(0);
            } else {
                if(ispipe==1) {

                    close(execpipe[0]);
                    write(execpipe[1],in,strlen(in));
                    close(execpipe[1]);
                }
            }

            int stat_loc;
            waitpid(p, &stat_loc, WUNTRACED);
        }
        close(connfd);
        if ((listen(sockfd, 5)) != 0) {
            printf("Listen failed...\n");
            exit(0);
        }

        len = sizeof(cli);
        connfd = accept(sockfd, (SA*)&cli, &len);
        if (connfd < 0) {
            printf("server acccept failed...\n");
            exit(0);
        }
    }


    return 0;
}
