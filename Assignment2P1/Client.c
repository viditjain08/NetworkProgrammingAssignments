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

char buffer1[MAXDATA][2*BLOCKSIZE];
char buffer2[MAXDATA][2*BLOCKSIZE];
int startptr1[MAXDATA];
int startptr2[MAXDATA];
int bufferselect[MAXDATA];
int currentptr1[MAXDATA];
int currentptr2[MAXDATA];
int currentselect[MAXDATA];

// typedef struct node * NODE;
// struct node {
//     char name[256];
//     NODE next;
// };
// NODE filestart;
// NODE filecur;
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

        while(startptr1[i]<BLOCKSIZE && currentptr1[i]>=startptr1[i]);
        int idx=0;
        // printf("(%s)\n",rbuf1+cur1);

        while(buffer1[i][currentptr1[i]]!='\0') {
            buf[idx] = buffer1[i][currentptr1[i]];
            idx++;
            currentptr1[i]++;
            if(currentptr1[i]>=startptr1[i] && startptr1[i]>BLOCKSIZE) {
                currentselect[i]=1;
                datacopyToString(buf+idx,i);
                return;
            }
            // printf("viditjain08-%c-%d-%d\n",buf[idx-1],currentptr1[i],startptr1[i]);

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

        while(buffer2[i][currentptr2[i]]!='\0') {
            buf[idx] = buffer2[i][currentptr2[i]];
            idx++;
            currentptr2[i]++;
            if(currentptr2[i]>=startptr2[i] && startptr2[i]>BLOCKSIZE) {
                currentselect[i]=0;
                datacopyToString(buf+idx,i);
                return;
            }
            while(currentptr2[i]>=startptr2[i]);
        }
        buf[idx]='\0';
        currentptr2[i]++;
        currentptr1[i]=0;
        if(currentptr2[i]>=startptr2[i] && startptr2[i]>BLOCKSIZE) {
            currentselect[i]=0;
        }
    }
    // printf("String: %s\n",buf);
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
                    // printf("Client received: %s\n",buffer1[i]+startptr1[i]);
                    startptr1[i]+=n;
                } else {
                    // buffer2[i][startptr2[i]+n]='\0';
                    // printf("Client received: %s\n",buffer2[i]+startptr2[i]);
                    startptr2[i]+=n;
                }
                // printf("CLIENT RECIEVED: (%d)\n", n);
            }
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

// int checkfileexists(char* file_name) {
//     NODE temp = filestart;
//     while(temp!=NULL) {
//         if(strcmp(temp->name, file_name)==0) {
//             return 1;
//         }
//         temp = temp->next;
//     }
//     return 0;
// }
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

    printf("No of parts: %d\n",no_of_parts);
    char strparts[10];
    sprintf(strparts, "%d", no_of_parts);
    write(sockfd,strparts,strlen(strparts)+1);
    int ptr=0;
    fseek(fp, 0L, SEEK_SET);
    char temp_buf[BLOCKSIZE];
    int fd = fileno(fp);
    // printf("Bytes read-%d\n",n);
    // while(checkfileexists(file_name)==1) {
    //     strcat(file_name, "new");
    // }
    // NODE new = (NODE)malloc(sizeof(struct node));
    // new->next = NULL;
    // strcpy(new->name,file_name);
    // filecur->next = new;
    // filecur = new;
    int count=0;
    // char dir[MAX_SIZE];
    // copyToString(dir);

    char *found;
    char dir[MAX_SIZE];
    found = strsep(&file_name,"/");
    while(found!=NULL) {
        strcpy(dir,found);
        found = strsep(&file_name,"/");
    }
    for(int i=0;i<no_of_parts;i++) {
        write(datafds[ptr],"0\0",2);
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

        // int x = write(datafds[ptr],temp_buf,n);
        write(datafds[ptr],"\0",1);
        // printf("-%d-\n",ret);

        write(datafds[ptr],dir,1+strlen(dir));

        // write(datafds[ptr],file_name,strlen(file_name));
        char strcount[10];
        sprintf(strcount, "%d", count);
        write(datafds[ptr],strcount,strlen(strcount));
        write(datafds[ptr],"\0",1);

        sprintf(strcount, "%d", i);
        write(datafds[ptr],strcount,strlen(strcount));
        write(datafds[ptr],"\0",1);

        char data_loc[MAX_SIZE];

        datacopyToString(data_loc, ptr);

        if(i==0 && count==0) {
            write(sockfd,data_loc,strlen(data_loc)+1);
            char *data_temp = (char*)malloc(sizeof(char)*MAX_SIZE);
            strcpy(data_temp,data_loc);
            char *found_dir;
            found_dir = strsep(&data_temp,"/");
            while(found_dir!=NULL) {
                strcpy(dir,found_dir);
                found_dir = strsep(&data_temp,"/");
            }
            printf("File Loc Name: %s\n",dir);
        }
        if(ptr==(no_of_dataservers-1)) {
            count++;
        }
        ptr = (ptr+1)%(no_of_dataservers);

    }
}

void printfile(char *file_name, int no_of_chunks) {
    int ptr=0;
    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"1\0",2);
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

        printf("%s",temp_buf);
        fflush(stdout);
        ptr = (ptr+1)%(no_of_dataservers);

    }
    printf("\n");
}

void savefile(char *name, char *file_name, int no_of_chunks) {
    int ptr=0;
    for(int i=0;i<no_of_chunks;i++) {
        write(datafds[ptr],"1\0",2);
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

        printf("%s",temp_buf);
        fflush(stdout);
        ptr = (ptr+1)%(no_of_dataservers);

    }
    printf("\n");
}
int main() {
    // filestart = (NODE)malloc(sizeof(struct node));
    // filestart->next = NULL;
    // strcpy(filestart->name,"home");
    // filecur = filestart;

    for(int i=0;i<MAXDATA;i++) {
        startptr1[i]=0;
        startptr2[i]=0;
        bufferselect[i]=0;
        currentptr1[i]=0;
        currentptr2[i]=0;
        currentselect[i]=0;
    }
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
        } else if(strcmp(found,"cd")==0) {
            write(sockfd,"2\0",2);
            found = strsep(&command," ");
            write(sockfd,found,strlen(found)+1);
            char choice[MAX_SIZE];
            copyToString(choice);
            if(strcmp(choice,"1")==0) {
                printf("Directory not present\n");
            }
        } else if(strcmp(found,"cat")==0) {
          printf("Printing File\n");
          write(sockfd,"5\0",2);
          found = strsep(&command," ");
          write(sockfd,found,strlen(found)+1);
          char temp[10];
          copyToString(temp);
          if(strcmp(temp,"0")==0) {
              printf("File not found in current directory\n");
              continue;
          }
          char loc[MAX_SIZE];
          copyToString(loc);
          char chunks[10];
          copyToString(chunks);
          printf("Location: %s\n",loc);
          printfile(loc,atoi(chunks));
        } else if(strcmp(found,"mv")==0) {

          printf("Moving File\n");
          write(sockfd,"4\0",2);
          found = strsep(&command," ");
          write(sockfd,found,strlen(found)+1);
          found = strsep(&command," ");
          write(sockfd,found,strlen(found)+1);
          char temp[MAX_SIZE];
          copyToString(temp);
          if(strcmp(temp,"1")==0) {
              printf("Invalid move\n");
              continue;
          }
        } else if(strcmp(found,"cp")==0) {
          printf("Copying file\n");
        } else if(strcmp(found,"rm")==0) {
          printf("Removing file\n");
        } else if(strcmp(found,"tobigfs")==0) {
          printf("Transferring to bigfs\n");
          write(sockfd,"3\0",2);


          found = strsep(&command," ");
          write(sockfd,found,strlen(found)+1);

          char temp[MAX_SIZE];
          copyToString(temp);
          if(strcmp(temp, "1")==0) {
              printf("File/Directory with the same name exists\n");
              continue;
          }
          transfertoBigFS(found);
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
