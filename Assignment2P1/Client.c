#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<stdbool.h>
#include<signal.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <dirent.h>
#define MAX_SIZE 1024
#define PORT1 8080
#define PORT3 8084
#define MAXDATA 10
#define BLOCKSIZE 1024000

#define SA struct sockaddr
char dataips[MAXDATA][MAX_SIZE];
int sockfd, connfd;
int no_of_dataservers = 0;
int datafds[MAXDATA];
char rbuf1[2*MAX_SIZE];
char rbuf2[2*MAX_SIZE];
int start1=0,start2=0,bufsel=0;
int cur1=0,cur2=0,cursel=0;
char *command;

char buffer1[MAXDATA][2*BLOCKSIZE] = {0};
char buffer2[MAXDATA][2*BLOCKSIZE] = {0};
int startptr1[MAXDATA] = {0};
int startptr2[MAXDATA] = {0};
int bufferselect[MAXDATA] = {0};
int currentptr1[MAXDATA] = {0};
int currentptr2[MAXDATA] = {0};
int currentselect[MAXDATA] = {0};

int is_regular_file(const char *path)
{
    struct stat path_stat;
    stat(path, &path_stat);
    return S_ISREG(path_stat.st_mode);
}

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

        while(startptr1[i]<BLOCKSIZE && currentptr1[i]>=startptr1[i]);
        int idx=0;
        // printf("(%s)\n",rbuf1+cur1);
        // printf("0-%d/%d\n",currentptr1[i],startptr1[i]);
        // while(currentptr1[i]<startptr1[i] && buffer1[i][currentptr1[i]]=='\0') {
        //     currentptr1[i]++;
        // }
        while(buffer1[i][currentptr1[i]]!='\0') {
            buf[idx] = buffer1[i][currentptr1[i]];
            idx++;
            currentptr1[i]++;
            if(currentptr1[i]>=startptr1[i] && startptr1[i]>BLOCKSIZE) {
                currentselect[i]=1;
                datacopyToString(buf+idx,i);
                return;
            }
            while(currentptr1[i]>=startptr1[i]);

        }

        buf[idx]='\0';
        currentptr1[i]++;
        currentptr2[i]=0;
        if(currentptr1[i]>=startptr1[i] && startptr1[i]>BLOCKSIZE) {
            currentselect[i]=1;
            // printf("fsbjbvvbkd7n-%d\n",start2);
        }
    } else {
        // printf("jain%d-%d\n",startptr1[i],currentptr1[i]);

        while(startptr2[i]<BLOCKSIZE && currentptr2[i]>=startptr2[i]);

        int idx=0;
        // printf("(-%s-)\n",rbuf2+cur2);
        // while(currentptr2[i]<startptr2[i] && buffer2[i][currentptr2[i]]=='\0') {
        //     currentptr2[i]++;
        // }
        while(buffer2[i][currentptr2[i]]!='\0') {
            buf[idx] = buffer2[i][currentptr2[i]];
            idx++;
            currentptr2[i]++;
            if(currentptr2[i]>=startptr2[i] && startptr2[i]>BLOCKSIZE) {
                currentselect[i]=0;
                datacopyToString(buf+idx,i);
                return;
            }
            // printf("1-%d/%d\n",currentptr2[i],startptr2[i]);

            while(currentptr2[i]>=startptr2[i]);

        }
        buf[idx]='\0';
        currentptr2[i]++;
        currentptr1[i]=0;
        if(currentptr2[i]>=startptr2[i] && startptr2[i]>BLOCKSIZE) {
            currentselect[i]=0;
        }
    }
}
void *readfromData(void *vargp) {
    int i = *((int*)vargp);
    int n;
    fd_set rset,wset;
    while(1) {
        FD_ZERO(&rset);
        FD_ZERO(&wset);
        FD_SET(datafds[i], &rset);
        select(datafds[i]+1,&rset,&wset,NULL,NULL);

        if(FD_ISSET(datafds[i], &rset)) {
        // if(1) {
            if(bufferselect[i]==0 && startptr1[i]>BLOCKSIZE) {
                bufferselect[i]=1;
                startptr2[i]=0;
            } else if(bufferselect[i]==1 && startptr2[i]>BLOCKSIZE) {
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
                    // printf("Client received: %d\n",n+startptr1[i]);
                    startptr1[i]+=n;
                } else {
                    // buffer2[i][startptr2[i]+n]='\0';
                    // printf("Client received: %d\n",n+startptr2[i]);
                    startptr2[i]+=n;
                }
                // printf("CLIENT RECIEVED: (%d)\n", n);
            }
        }

    }
}



void listDirectory() {
    write(sockfd,"ls",3);
    int n;
    int start=0;
    char readbuffer[MAX_SIZE];
    copyToString(readbuffer);
    printf("%s",readbuffer);
}

void makeDirectory() {
    char *commandfound;
    commandfound = strsep(&command," ");
    if(commandfound==NULL) {
        printf("No directory Specified\n");
        return;
    }
    write(sockfd,"mkdir",6);
    write(sockfd,commandfound,strlen(commandfound)+1);
}

void changeDirectory() {
    write(sockfd,"cd",3);
    char *commandfound;
    commandfound = strsep(&command," ");
    if(commandfound==NULL) {
        printf("No directory Specified\n");
        return;
    }
    write(sockfd,commandfound,strlen(commandfound)+1);
    char choice[MAX_SIZE];
    copyToString(choice);
    if(strcmp(choice,"1")==0) {
        printf("Directory not present\n");
    }
}

void transfertoBigFS(char* file_name) {
    FILE* fp = fopen(file_name, "r");

    // checking if the file exist or not
    if (fp == NULL) {
        printf("File Not Found!\n");
        return;
    }

    fseek(fp, 0L, SEEK_END);

    // calculating the size of the file
    long int res = ftell(fp);
    int no_of_parts = 1+res/(BLOCKSIZE);

    char strparts[10];
    sprintf(strparts, "%d", no_of_parts);
    write(sockfd,strparts,strlen(strparts)+1);
    int ptr=0;
    fseek(fp, 0L, SEEK_SET);
    char temp_buf[BLOCKSIZE];
    int fd = fileno(fp);

    int count=0;

    char *found;
    char dir[MAX_SIZE];
    found = strsep(&file_name,"/");
    while(found!=NULL) {
        strcpy(dir,found);
        found = strsep(&file_name,"/");
    }
    for(int i=0;i<no_of_parts;i++) {
        write(datafds[ptr],"tobigfs",8);
        int n = read(fd, temp_buf, sizeof(temp_buf));
        int write_count=0;
        fd_set rset, wset;

        while(write_count<n) {
            FD_ZERO(&rset);
            FD_ZERO(&wset);
            FD_SET(datafds[ptr], &wset);
            select(datafds[ptr]+1,&rset,&wset,NULL,NULL);
            if(FD_ISSET(datafds[ptr],&wset)) {
                int x;
                if(write_count+(BLOCKSIZE/1000)>=n)
                    x = write(datafds[ptr],temp_buf+write_count,n-write_count);
                else
                    x = write(datafds[ptr],temp_buf+write_count,BLOCKSIZE/1000);
                write_count+=(BLOCKSIZE)/1000;
                // printf("Write count: %d-%d-%d\n",write_count,x,errno);
            }

        }

        write(datafds[ptr],"\0",1);

        write(datafds[ptr],dir,1+strlen(dir));

        char strcount[10];
        sprintf(strcount, "%d", count);
        write(datafds[ptr],strcount,1+strlen(strcount));

        sprintf(strcount, "%d", i);
        write(datafds[ptr],strcount,1+strlen(strcount));

        char data_loc[MAX_SIZE];

        datacopyToString(data_loc, ptr);

        if(i==0) {
            write(sockfd,data_loc,strlen(data_loc)+1);
            char *data_temp = (char*)malloc(sizeof(char)*MAX_SIZE);
            strcpy(data_temp,data_loc);
            char *found_dir;
            found_dir = strsep(&data_temp,"/");
            while(found_dir!=NULL) {
                strcpy(dir,found_dir);
                found_dir = strsep(&data_temp,"/");
            }
            // printf("File Loc Name: %s\n",dir);
        }
        if(ptr==(no_of_dataservers-1)) {
            count++;
        }
        ptr = (ptr+1)%(no_of_dataservers);

    }
}

void tobigfs() {
    write(sockfd,"tobigfs",8);

    char *commandfound;
    commandfound = strsep(&command," ");
    write(sockfd,commandfound,strlen(commandfound)+1);

    char temp[MAX_SIZE];
    copyToString(temp);
    if(strcmp(temp, "1")==0) {
      printf("File/Directory with the same name exists\n");
      return;
    }
    transfertoBigFS(commandfound);
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


void printfile(char *file_name, int no_of_chunks) {
    int ptr=0;
    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"cat",4);
        write(datafds[ptr],file_name,strlen(file_name));
        char chunks[10];
        sprintf(chunks,"%d",i);
        write(datafds[ptr],chunks,strlen(chunks)+1);
        ptr = (ptr+1)%(no_of_dataservers);
    }
    ptr = 0;
    for(int i=0;i<no_of_chunks;i++) {
        char temp_buf[BLOCKSIZE];

        datacopyToString(temp_buf, ptr);
        while(strlen(temp_buf)!=0) {
            printf("%s",temp_buf);
            datacopyToString(temp_buf, ptr);
        }
        ptr = (ptr+1)%(no_of_dataservers);

    }
    printf("\n");
}

void savefile() {
    // printf("Transferring from bigfs\n");
    write(sockfd,"frombigfs",10);
    char *found;
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    char temp[10];
    copyToString(temp);
    if(strcmp(temp,"0")==0) {
        printf("File not found in current directory\n");
        return;
    }
    char loc[MAX_SIZE];
    copyToString(loc);
    char chunks[10];
    copyToString(chunks);
    // printf("Location: %s\n",loc);
    int no_of_chunks = atoi(chunks);
    char *name = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(name,found);
    char *file_name = (char*)malloc(sizeof(char)*MAX_SIZE);
    strcpy(file_name,loc);
    char found2[MAX_SIZE];
    found = strsep(&name,"/");
    while(found!=NULL) {
        strcpy(found2,found);
        found = strsep(&name,"/");
    }

    int ptr=0;
    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"cat",4);
        write(datafds[ptr],file_name,strlen(file_name));
        char chunks[10];
        sprintf(chunks,"%d",i);
        write(datafds[ptr],chunks,strlen(chunks)+1);
        ptr = (ptr+1)%(no_of_dataservers);
    }
    FILE *fp = fopen(found2, "w");
    int fd = fileno(fp);
    ptr = 0;
    for(int i=0;i<no_of_chunks;i++) {

        char temp_buf[BLOCKSIZE];

        datacopyToString(temp_buf, ptr);
        while(strlen(temp_buf)!=0) {
            fprintf(fp,"%s",temp_buf);
            datacopyToString(temp_buf, ptr);
        }


        ptr = (ptr+1)%(no_of_dataservers);

    }
    write(fd,"\0",1);
    fclose(fp);

}

void removefile() {
    write(sockfd,"rm",3);
    char *found;
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    char temp[10];
    copyToString(temp);
    if(strcmp(temp,"0")==0) {
        printf("File not found in current directory\n");
        return;
    }
    char loc[MAX_SIZE];
    copyToString(loc);
    char chunks[10];
    copyToString(chunks);
    // printf("Location: %s\n",loc);
    int no_of_chunks = atoi(chunks);
    int ptr=0;
    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"rm",3);
        write(datafds[ptr],loc,strlen(loc)+1);
        char chunks[10];
        sprintf(chunks,"%d",i);
        write(datafds[ptr],chunks,strlen(chunks)+1);
        ptr = (ptr+1)%(no_of_dataservers);
    }
}

void copyfile() {
    write(sockfd,"cp",3);
    char *found;
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    char temp[MAX_SIZE];
    copyToString(temp);
    if(strcmp(temp,"1")==0) {
      printf("Invalid copy\n");
      return;
    }
    char loc[MAX_SIZE];
    copyToString(loc);
    char chunks[10];
    copyToString(chunks);
    // printf("Location: %s\n",loc);
    int no_of_chunks = atoi(chunks);
    int ptr=0;
    char loca[MAX_SIZE];

    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"cp",3);
        write(datafds[ptr],loc,strlen(loc)+1);
        char chunks[10];
        sprintf(chunks,"%d",i);
        write(datafds[ptr],chunks,strlen(chunks)+1);
        if(i==0) {
            datacopyToString(loca, ptr);
            write(sockfd,loca,strlen(loca)+1);
            // strcpy(loc,loca);
        } else {
            write(datafds[ptr],loca,strlen(loca)+1);
        }
        ptr = (ptr+1)%(no_of_dataservers);
    }
}

void catfile() {
    write(sockfd,"cat",4);
    char *found;
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    char temp[10];
    copyToString(temp);
    if(strcmp(temp,"0")==0) {
      printf("File not found in current directory\n");
      return;
    }
    char loc[MAX_SIZE];
    copyToString(loc);
    char chunks[10];
    copyToString(chunks);
    // printf("Location: %s\n",loc);
    printfile(loc,atoi(chunks));
}

void movefile() {
    write(sockfd,"mv",3);
    char *found;
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    found = strsep(&command," ");
    write(sockfd,found,strlen(found)+1);
    char temp[MAX_SIZE];
    copyToString(temp);
    if(strcmp(temp,"1")==0) {
      printf("Invalid move\n");
      return;
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
    FILE *t = fopen("server_address","r");
    char nameServer[20];
    fscanf(t,"%s", nameServer);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(nameServer);
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
    printf("Data Servers' ip addresses-\n");
    while(temp!=NULL) {
        if(strlen(temp)==0) {
            break;
        }
        strcpy(dataips[no_of_dataservers],temp);
        printf("%s\n",dataips[no_of_dataservers]);
        temp = strsep(&y," ");
        no_of_dataservers++;
    }
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
        command = (char*)malloc(sizeof(char)*MAX_SIZE);
        char *found;
        fgets(command, MAX_SIZE, stdin);
        command[strlen(command)-1]='\0';
        found = strsep(&command," ");
        if(found==NULL) {
            printf("Invalid  Command\n");
            continue;
        }
        if(strcmp(found, "ls")==0) {

            listDirectory();

        } else if(strcmp(found, "mkdir")==0) {

            makeDirectory();

        } else if(strcmp(found,"cd")==0) {

            changeDirectory();

        } else if(strcmp(found,"cat")==0) {

            catfile();

        } else if(strcmp(found,"mv")==0) {

            movefile();

        } else if(strcmp(found,"cp")==0) {

            copyfile();

        } else if(strcmp(found,"rm")==0) {

            removefile();

        } else if(strcmp(found,"tobigfs")==0) {

            tobigfs();

        } else if(strcmp(found,"frombigfs")==0) {

            savefile();

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
