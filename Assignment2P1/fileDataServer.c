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
#include <fcntl.h>
#include<pthread.h>

#define MAX_SIZE 1024
#define PORT2 8082
#define PORT3 8084
#define SA struct sockaddr

int clientfd,clientconn;

char rbuf1[2*MAX_SIZE];
char rbuf2[2*MAX_SIZE];
int start1=0,start2=0,bufsel=0;
int cur1=0,cur2=0,cursel=0;
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
    // printf("String: %s\n",buf);
}
void *readfromClient(void *vargp) {
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
            n = read(clientconn, rbuf1+start1, sizeof(rbuf1)-start1);
        else
            n = read(clientconn, rbuf2+start2, sizeof(rbuf2)-start2);
        if(n<0) {
            if(errno != EWOULDBLOCK) {
                printf("Read error\n");
            }
        }
        else if(n==0) {

        } else {
            if(bufsel==0) {
                start1+=n;
            } else {
                start2+=n;
            }
            printf("CLIENT RECIEVED: %d\n", n);
        }
    }
}

void connectToClient() {
    struct sockaddr_in servaddr, cli;

    // socket create and verification
    clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if (clientfd == -1) {
      printf("Client socket creation failed...\n");
      exit(0);
    }
    else
      printf("Client Socket successfully created..\n");
    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT3);
    if (setsockopt(clientfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(0);
    }
    // Binding newly created socket to given IP and verification
    if ((bind(clientfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
      printf("Client socket bind failed...\n");
      exit(0);
    }
    else
      printf("Client Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(clientfd, 5)) != 0) {

    }
    else
      printf("Server listening..\n");
    int len = sizeof(cli);

    // Accept the data packet from client and verification
    clientconn = accept(clientfd, (SA*)&cli, &len);
    if (clientconn < 0) {
      printf("server acccept failed...\n");
      exit(0);
    }
    else
      printf("server acccept the client...\n");
    int flags = fcntl(clientconn, F_GETFL, 0);

    fcntl(clientconn, F_SETFL, flags | O_NONBLOCK);
}
int main() {

    int sockfd;
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
    servaddr.sin_port = htons(PORT2);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    connectToClient();
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, readfromClient, NULL);
    while(1);

}
