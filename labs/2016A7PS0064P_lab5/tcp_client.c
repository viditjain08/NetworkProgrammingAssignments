#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

#define MAX_SIZE 1024

int main(){
	char buffer[MAX_SIZE];
	char url[MAX_SIZE];
	int port;
	printf("Enter URL\n");
	scanf("%s", url);
	printf("Enter port number\n");
	scanf("%d", &port);

    struct hostent *ip;
	struct sockaddr_in addr;
	int start = 1, sock;

    time_t begin = clock();

    if((ip = gethostbyname(url)) == NULL){
		herror("gethostbyname");
		exit(1);
	}

 	bcopy(ip->h_addr, &addr.sin_addr, ip->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	setsockopt(sock, IPPROTO_TCP, TCP_NODELAY, (const char *)&start, sizeof(int));

	if(sock == -1){
		perror("setsockopt");
		exit(1);
	}

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);

	}
	int fd = sock;
	write(fd, "GET /\r\n\r\n", strlen("GET /\r\n\r\n"));
	bzero(buffer, MAX_SIZE);

	while(read(fd, buffer, MAX_SIZE - 1) != 0){
		fprintf(stderr, "%s", buffer);
		bzero(buffer, MAX_SIZE);
	}

	shutdown(fd, SHUT_RDWR);
	close(fd);
    sleep(1);
    time_t end = clock();
    printf("\nTime: %f seconds\n", (double)(end-begin)/CLOCKS_PER_SEC);
	return 0;
}
