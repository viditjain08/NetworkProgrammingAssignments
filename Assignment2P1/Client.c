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
#include <fcntl.h>
#include <arpa/inet.h>
#define MAX_SIZE 1024
#define PORT1 8080
#define MAXDATA 30
#define SA struct sockaddr
char dataips[MAXDATA][MAX_SIZE];
int sockfd, connfd;

void listDirectory() {
    int n;
    int start=0;
    char readbuffer[1024];
    while(1) {
        n = read(sockfd, readbuffer+start, sizeof(readbuffer)-start);
        if(n<0) {
            if(errno != EWOULDBLOCK) {
                printf("Read error\n");
            }
        }
        else if(n==0) {
            break;
        } else {
            start+=n;
            // printf("CLIENT RECIEVED: %d\n", n);
            if(readbuffer[start-1]=='\0') {
                break;
            }
        }
    }
    printf("%s",readbuffer);
}


int main() {

    struct sockaddr_in servaddr, cli;

    // socket create and varification
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("socket creation failed...\n");
        exit(0);
    }
    else
        printf("Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr("172.17.72.164");
    servaddr.sin_port = htons(PORT1);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    int x=fcntl(sockfd ,F_GETFL, 0);
    fcntl(sockfd, F_SETFL, x | O_NONBLOCK);
    int n;
    char readbuffer[MAX_SIZE];
    int start = 0;
    while(1) {
        n = read(sockfd, readbuffer+start, sizeof(readbuffer)-start);
        if(n<0) {
            if(errno != EWOULDBLOCK) {
                printf("Read error\n");
            }
        }
        else if(n==0) {
            break;
        } else {
            start+=n;
            printf("CLIENT RECIEVED: %d\n", n);
            if(readbuffer[start-1]=='\0') {
                break;
            }
        }
    }
    readbuffer[start]='\0';
    // printf("Client: %s\n",readbuffer);
    char *y = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(y,readbuffer);

    char *temp;
    temp = strsep(&y," ");
    int i=0;
    while(temp!=NULL) {
        strcpy(dataips[i],temp);
        printf("%s\n",dataips[i]);
        temp = strsep(&y," ");
        i++;
    }

    while(1) {
        printf("prompt> ");
        fflush(stdout);
        char *command = (char*)malloc(sizeof(char)*MAX_SIZE);
        char *found;
        fgets(command, MAX_SIZE, stdin);
        command[strlen(command)-1]='\0';
        found = strsep(&command," ");
        if(found==NULL) {
          printf("Invalid  Command\n");
          continue;
        }
        if(strcmp(found, "ls")==0) {
          write(sockfd,"0\0",2);
          listDirectory();
        } else if(strcmp(found, "mkdir")==0) {
          write(sockfd,"1\0",2);
          found = strsep(&command," ");
          write(sockfd,found,strlen(found)+1);
        } else if(strcmp(found,"cat")==0) {
          printf("Printing File\n");
        } else if(strcmp(found,"mv")==0) {
          printf("Moving File\n");
        } else if(strcmp(found,"cp")==0) {
          printf("Copying file\n");
        } else if(strcmp(found,"rm")==0) {
          printf("Removing file\n");
        } else if(strcmp(found,"tobigfs")==0) {
          printf("Transferring to bigfs\n");
        } else if(strcmp(found,"frombigfs")==0) {
          printf("Transferring from bigfs\n");
        } else if(strcmp(found,"exit")==0) {
          printf("Exiting\n");
          close(sockfd);
          exit(0);
        }
        else {
          printf("Invalid command\n");
        }
    }

}
