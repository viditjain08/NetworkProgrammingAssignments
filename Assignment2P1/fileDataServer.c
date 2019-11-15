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
#define PORT2 8082
#define PORT3 8084
#define SA struct sockaddr

int clientfd,clientconn;
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
    len = sizeof(cli);

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
    while(1);

}
