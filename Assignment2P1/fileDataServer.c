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
#include <sys/stat.h>
#include<unistd.h>

#define BLOCKSIZE 1024000
#define MAX_SIZE 1024
#define PORT2 8082
#define PORT3 8084
#define SA struct sockaddr

int clientfd,clientconn;

char rbuf1[2*BLOCKSIZE];
char rbuf2[2*BLOCKSIZE];
int start1=0,start2=0,bufsel=0;
int cur1=0,cur2=0,cursel=0;
void copyToString(char *buf) {
    if(cursel==0) {
        while(start1<BLOCKSIZE && cur1>=start1);
        int i=0;
        // printf("(%s)\n",rbuf1+cur1);

        while(rbuf1[cur1]!='\0') {
            buf[i] = rbuf1[cur1];
            i++;
            cur1++;
            if(cur1>=start1 && start1>BLOCKSIZE) {
                cursel=1;
                copyToString(buf+i);
                return;
            }
            while(cur1>=start1);
        }
        buf[i]='\0';
        cur1++;
        cur2=0;
        if(cur1>=start1 && start1>BLOCKSIZE) {
            cursel=1;
            // printf("fsbjbvvbkd7n-%d\n",start2);
        }
    } else {
        while(start2<BLOCKSIZE && cur2>=start2);

        int i=0;
        // printf("(-%s-)\n",rbuf2+cur2);

        while(rbuf2[cur2]!='\0') {
            buf[i] = rbuf2[cur2];
            i++;
            cur2++;
            if(cur2>=start2 && start2>BLOCKSIZE) {
                cursel=0;
                copyToString(buf+i);
                return;
            }
            while(cur2>=start2);
        }
        buf[i]='\0';
        cur2++;
        cur1=0;
        if(cur2>=start2 && start2>BLOCKSIZE) {
            cursel=0;
        }
    }
    // printf("String: %s\n",buf);
}
void *readfromClient(void *vargp) {
    int n;
    while(1) {
        if(bufsel==0 && start1>BLOCKSIZE) {
            bufsel=1;
            start2=0;
        } else if(bufsel==1 && start2>BLOCKSIZE) {
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
            // printf("CLIENT RECIEVED: %d\n", n);
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

void printfile() {
    char data_loc[MAX_SIZE];
    copyToString(data_loc);
    // printf("Data location : %s\n",data_loc);
    FILE* fp = fopen(data_loc, "r");
    if (fp == NULL) {
        printf("fopen failed, errno = %d\n", errno);
    }

    int fd = fileno(fp);
    char temp_buf[BLOCKSIZE];
    int n = read(fd, temp_buf, BLOCKSIZE);
    printf("Data read %d bytes\n",n);
    int write_count=0;
    fd_set rset, wset;

    while(write_count<n) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_SET(clientconn, &wset);
        select(clientconn+1,&rset,&wset,NULL,NULL);
        if(FD_ISSET(clientconn,&wset)) {
            int x;
            if(write_count+(BLOCKSIZE/1000)>=n) {
                x = write(clientconn,temp_buf+write_count,n-write_count);
                write(clientconn,"\0\0",2);
            }
            else {
                x = write(clientconn,temp_buf+write_count,BLOCKSIZE/1000);
                write(clientconn,"\0",1);
            }
            write_count+=(BLOCKSIZE/1000);
            // printf("Write count: %d-%d-%d\n",write_count,x,errno);
        }

    }
}

void tobigfs() {
    char temp_buf[2*BLOCKSIZE];
    copyToString(temp_buf);

    char name[MAX_SIZE];
    copyToString(name);
    printf("%s\n",name);
    char dir[MAX_SIZE] = "data/";

    char count[10];
    copyToString(count);

    char num[20];
    copyToString(num);
    printf("%s\n",num);
    strcat(dir,name);
    if(strcmp(count,"0")==0 && access( dir, F_OK ) != -1 ) {
        int i=0;
        char str[MAX_SIZE];
        sprintf(str, "%s(%d)", dir,i);
        // printf("%s\n",str);
        while(access(str,F_OK)!=-1) {
            i++;
            sprintf(str, "%s(%d)", dir,i);
        }
        strcpy(dir,str);
    }
    // printf("Directory: %s\n",dir);

    FILE *fp = fopen(dir,"w");
    fclose(fp);
    write(clientconn,dir,strlen(dir)+1);
    strcat(dir,num);
    fp = fopen(dir, "w");

    fputs(temp_buf, fp);
    fclose(fp);
}

void removefile() {
    char name[MAX_SIZE];
    copyToString(name);
    char chunks[10];
    copyToString(chunks);
    if(access(name,F_OK)!=-1) {
        remove(name);
    }
    strcat(name,chunks);
    if (remove(name) != 0)
        printf("Unable to delete the file");
}

void copyfile() {
    char name[MAX_SIZE];
    copyToString(name);
    char chunks[10];
    copyToString(chunks);
    int i=0;
    char str[MAX_SIZE];
    sprintf(str, "%s(%d)", name,i);
    // printf("%s\n",str);
    while(strcmp(chunks,"0")==0 && access(str,F_OK)!=-1) {
        i++;
        sprintf(str, "%s(%d)", name,i);
    }
    char source[MAX_SIZE];
    strcpy(source,name);
    if(strcmp(chunks,"0")==0) {
        strcpy(name,str);
        write(clientconn,name,strlen(name)+1);
    } else {
        copyToString(name);
    }
    // printf("File nae: %s\n",source);

    char ch;
    strcat(source,chunks);

    FILE* source1 = fopen(source, "r");

    if( source1 == NULL )
    {
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }
    FILE* tempfile = fopen(name,"w");
    fclose(tempfile);
    strcat(name,chunks);
    FILE* target = fopen(name, "w");

    if( target == NULL )
    {
        fclose(source1);
        printf("Press any key to exit...\n");
        exit(EXIT_FAILURE);
    }

    while( ( ch = fgetc(source1) ) != EOF )
        fputc(ch, target);

    // printf("File copied successfully.\n");
    fclose(source1);
    fclose(target);
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

    FILE *t = fopen("server_address","r");
    char nameServer[20];
    fscanf(t,"%s", nameServer);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(nameServer);
    servaddr.sin_port = htons(PORT2);

    // connect the client socket to server socket
    if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr)) != 0) {
        printf("connection with the server failed...\n");
        exit(0);
    }
    else
        printf("connected to the server..\n");
    connectToClient();
    int check;
    char* dirname = "data";
    check = mkdir(dirname,0777);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, readfromClient, NULL);

    while(1) {
        char choice[MAX_SIZE];
        copyToString(choice);
        if(strcmp(choice, "tobigfs")==0) {

            tobigfs();

        } else if(strcmp(choice,"cat")==0) {

            printfile();

        } else if(strcmp(choice,"rm")==0) {

            removefile();

        } else if(strcmp(choice,"cp")==0) {
            copyfile();
        }


    }

}
