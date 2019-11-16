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
#include <pthread.h>

#define MAX_SIZE 1024
#define PORT1 8080
#define PORT3 8084
#define MAXDATA 30
#define SA struct sockaddr
char dataips[MAXDATA][MAX_SIZE];
int sockfd, connfd;
int no_of_dataservers = 0;
int datafds[MAXDATA];
char rbuf1[2*MAX_SIZE];
char rbuf2[2*MAX_SIZE];
int start1=0,start2=0,bufsel=0;
int cur1=0,cur2=0,cursel=0;

char buffer1[MAXDATA][2*MAX_SIZE];
char buffer2[MAXDATA][2*MAX_SIZE];
int startptr1[MAXDATA] = {0};
int startptr2[MAXDATA] = {0};
int bufferselect[MAXDATA] = {0};
int currentptr1[MAXDATA] = {0};
int currentptr2[MAXDATA] = {0};
int currentselect[MAXDATA] = {0};
void copyToString(char *buf) {
    if(cursel==0) {
        while(start1<MAX_SIZE && cur1>=start1);
        int i=0;
        // printf("(%s)\n",rbuf1+cur1);

        while(rbuf1[cur1]!='\0') {
            buf[i] = rbuf1[cur1];
            i++;
            cur1++;
            if(cur1>=start1 && start1>MAX_SIZE) {
                cursel=1;
                copyToString(buf+i);
                return;
            }
            while(cur1>=start1);
        }
        buf[i]='\0';
        cur1++;
        cur2=0;
        if(cur1>=start1 && start1>MAX_SIZE) {
            cursel=1;
            // printf("fsbjbvvbkd7n-%d\n",start2);
        }
    } else {
        while(start2<MAX_SIZE && cur2>=start2);

        int i=0;
        // printf("(-%s-)\n",rbuf2+cur2);

        while(rbuf2[cur2]!='\0') {
            buf[i] = rbuf2[cur2];
            i++;
            cur2++;
            if(cur2>=start2 && start2>MAX_SIZE) {
                cursel=0;
                copyToString(buf+i);
                return;
            }
            while(cur2>=start2);
        }
        buf[i]='\0';
        cur2++;
        cur1=0;
        if(cur2>=start2 && start2>MAX_SIZE) {
            cursel=0;
        }
    }
    printf("String: %s\n",buf);
}
void *readfromName(void *vargp) {
    int n;
    while(1) {
        if(bufsel==0 && start1>MAX_SIZE) {
            bufsel=1;
            start2=0;
        } else if(bufsel==1 && start2>MAX_SIZE) {
            bufsel=0;
            start1=0;
        }
        if(bufsel==0)
            n = read(sockfd, rbuf1+start1, sizeof(rbuf1)-start1);
        else
            n = read(sockfd, rbuf2+start2, sizeof(rbuf2)-start2);
        if(n<0) {
            if(errno != EWOULDBLOCK) {
                printf("Read error\n");
            }
        }
        else if(n==0) {

        } else {
            if(bufsel==0) {
                // rbuf1[start1+n]='\0';
                // printf("Client received: %s\n",rbuf1+start1);
                start1+=n;
            } else {
                // rbuf2[start2+n]='\0';
                // printf("Client received: %s\n",rbuf2+start2);
                start2+=n;
            }
            // printf("CLIENT RECIEVED: %s(%d)\n", rbuf1+start1-n, n);
        }
    }
}



void datacopyToString(char *buf, int i) {
    if(currentselect[i]==0) {
        while(startptr1[i]<MAX_SIZE && currentptr1[i]>=startptr1[i]);
        int idx=0;
        // printf("(%s)\n",rbuf1+cur1);

        while(buffer1[i][currentptr1[i]]!='\0') {
            buf[idx] = buffer1[i][currentptr1[i]];
            idx++;
            currentptr1[i]++;
            if(currentptr1[i]>=startptr1[i] && startptr1[i]>MAX_SIZE) {
                currentselect[i]=1;
                datacopyToString(buf+idx,i);
                return;
            }
            while(currentptr1[i]>=startptr1[i]);
        }
        buf[idx]='\0';
        currentptr1[i]++;
        currentptr2[i]=0;
        if(currentptr1[i]>=startptr1[i] && startptr1[i]>MAX_SIZE) {
            currentselect[i]=1;
            // printf("fsbjbvvbkd7n-%d\n",start2);
        }
    } else {
        while(startptr2[i]<MAX_SIZE && currentptr2[i]>=startptr2[i]);

        int idx=0;
        // printf("(-%s-)\n",rbuf2+cur2);

        while(buffer2[i][currentptr2[i]]!='\0') {
            buf[idx] = buffer2[i][currentptr2[i]];
            idx++;
            currentptr2[i]++;
            if(currentptr2[i]>=startptr2[i] && startptr2[i]>MAX_SIZE) {
                currentselect[i]=0;
                datacopyToString(buf+idx,i);
                return;
            }
            while(currentptr2[i]>=startptr2[i]);
        }
        buf[idx]='\0';
        currentptr2[i]++;
        currentptr1[i]=0;
        if(currentptr2[i]>=startptr2[i] && startptr2[i]>MAX_SIZE) {
            currentselect[i]=0;
        }
    }
    printf("String: %s\n",buf);
}
void *readfromData(void *vargp) {
    int i = *((int*)vargp);
    int n;
    while(1) {
        if(bufferselect[i]==0 && startptr1[i]>MAX_SIZE) {
            bufferselect[i]=1;
            startptr2[i]=0;
        } else if(bufferselect[i]==1 && startptr2[i]>MAX_SIZE) {
            bufferselect[i]=0;
            startptr1[i]=0;
        }
        if(bufferselect[i]==0)
            n = read(datafds[i], buffer1[i]+startptr1[i], sizeof(buffer1[i])-startptr1[i]);
        else
            n = read(datafds[i], buffer2[i]+startptr2[i], sizeof(buffer2[i])-startptr2[i]);
        if(n<0) {
            if(errno != EWOULDBLOCK) {
                printf("Read error\n");
            }
        }
        else if(n==0) {

        } else {
            if(bufferselect[i]==0) {
                // buffer1[i][startptr1[i]+n]='\0';
                // printf("Client received: %s\n",buffer1[i]+startptr1[i]);
                startptr1[i]+=n;
            } else {
                // buffer2[i][startptr2[i]+n]='\0';
                // printf("Client received: %s\n",buffer2[i]+startptr2[i]);
                startptr2[i]+=n;
            }
            // printf("CLIENT RECIEVED: %s(%d)\n", rbuf1+start1-n, n);
        }
    }
}



void listDirectory() {
    int n;
    int start=0;
    char readbuffer[1024];
    copyToString(readbuffer);
    printf("%s",readbuffer);
}

void connectToData() {
    for(int i=0;i<no_of_dataservers;i++) {
        struct sockaddr_in servaddr, cli;

        // socket create and varification
        datafds[i] = socket(AF_INET, SOCK_STREAM, 0);
        if (datafds[i] == -1) {
            printf("Data socket creation failed...\n");
            exit(0);
        }
        else
            printf("Socket successfully created..\n");
        bzero(&servaddr, sizeof(servaddr));

        // assign IP, PORT
        servaddr.sin_family = AF_INET;
        servaddr.sin_addr.s_addr = inet_addr(dataips[i]);
        servaddr.sin_port = htons(PORT3);

        // connect the client socket to server socket
        if (connect(datafds[i], (SA*)&servaddr, sizeof(servaddr)) != 0) {
            printf("connection with the Data server failed...\n");
            exit(0);
        }
        else
            printf("connected to the Data server..\n");
        int x=fcntl(datafds[i] ,F_GETFL, 0);
        fcntl(datafds[i], F_SETFL, x | O_NONBLOCK);

    }
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
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, readfromName, NULL);
    int n;
    char readbuffer[MAX_SIZE];
    int start = 0;
    copyToString(readbuffer);
    // printf("Client: %s\n",readbuffer);
    char *y = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(y,readbuffer);

    char *temp;
    temp = strsep(&y," ");
    while(temp!=NULL) {
        if(strlen(temp)==0) {
            break;
        }
        strcpy(dataips[no_of_dataservers],temp);
        printf("%s\n",dataips[no_of_dataservers]);
        temp = strsep(&y," ");
        no_of_dataservers++;
    }
    printf("%d\n",no_of_dataservers);
    connectToData();
    pthread_t threads[MAXDATA];
    for(int i=0;i<no_of_dataservers;i++) {
        int *arg = malloc(sizeof(*arg));
        *arg = i;
        pthread_create(&threads[i], NULL, readfromData, arg);

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
          pthread_cancel(thread_id);
          close(sockfd);
          exit(0);
        }
        else {
          printf("Invalid command\n");
        }
    }

}
