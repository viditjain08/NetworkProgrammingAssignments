#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // for close
#define MAX 80
#define PORT1 8080
#define SA struct sockaddr
#include <signal.h>

typedef struct node * NODE;
struct node {
    char name[256];
    NODE next;
    NODE child;
};

// Driver function
int flag=0;
void chldhandler(int sig) {
    flag=1;
}
int main()
{
    signal(SIGCHLD,chldhandler);
    int p = fork();
    if(p==0) {
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

    	// assign IP, PORT
    	servaddr.sin_family = AF_INET;
    	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    	servaddr.sin_port = htons(PORT1);

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
    } else {
        while(1) {
            if(flag==1) {
                flag=0;
                printf("vidit\n");
                break;
            }
        }
    }
    NODE home = (NODE)malloc(sizeof(struct node));
    home->child = NULL;
    strcpy(home->name,"home");
    home->next = NULL;
    NODE cur = home;
    // p = fork();
    // if(p==0) {
    //
    // }


}
