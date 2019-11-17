#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>  //Header file for sleep(). man 3 sleep for details.
#include <pthread.h>
#include <fcntl.h>
#include <arpa/inet.h>
#define MAX 80
#define MAX_SIZE 1024

#define PORT1 8080
#define PORT2 8082
#define MAXDATA 30
#define SA struct sockaddr
#include <signal.h>
#include <errno.h>

typedef struct node * NODE;
struct node {
    char name[256];
    NODE next;
    NODE child;
    NODE parent;
    char type[20];
    int no_of_chunks;
    char loc_name[MAX_SIZE];
};

// Driver function
int flag=0;
int sockfd, connfd, len;
int datafd;
int dataconnfd[MAXDATA];
struct sockaddr_in clients[MAXDATA];
int dataptr=0;

void connectToData() {

    datafd = socket(AF_INET, SOCK_STREAM, 0);
    if (datafd == -1) {
        printf("Data Socket could not be created\n");
        exit(0);
    }
    else
        printf("Data Socket successfully created..\n");
    int flags = fcntl(datafd, F_GETFL, 0);

    fcntl(datafd, F_SETFL, flags | O_NONBLOCK);
    if (setsockopt(datafd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(0);
    }

    struct sockaddr_in servaddr;

    // socket create and verification

    bzero(&servaddr, sizeof(servaddr));

    // assign IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT2);

    // Binding newly created socket to given IP and verification
    if ((bind(datafd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
      printf("Data socket bind failed...\n");
      exit(0);
    }
    else
      printf("Data Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(datafd, MAXDATA)) != 0) {

    }
    else
      printf("Data Server listening..\n");


}
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
            n = read(connfd, rbuf1+start1, sizeof(rbuf1)-start1);
        else
            n = read(connfd, rbuf2+start2, sizeof(rbuf2)-start2);
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

void *connectToClient(void *vargp) {
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
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){ 1 }, sizeof(int)) < 0) {
        printf("setsockopt(SO_REUSEADDR) failed\n");
        exit(0);
    }
    // Binding newly created socket to given IP and verification
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
      printf("socket bind failed...\n");
      exit(0);
    }
    else
      printf("Socket successfully binded..\n");

    // Now server is ready to listen and verification
    if ((listen(sockfd, 5)) != 0) {

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
    int flags = fcntl(connfd, F_GETFL, 0);

    fcntl(connfd, F_SETFL, flags | O_NONBLOCK);
    flag=1;
}

void sendiptoClient() {
    for(int i=0;i<dataptr;i++) {
        char str[INET_ADDRSTRLEN];
        inet_ntop( AF_INET, &clients[i].sin_addr, str, INET_ADDRSTRLEN );
        printf("%s\n",str);
        write(connfd, str, strlen(str));
        write(connfd," ",1);
    }
    write(connfd,"\0",1);
}

int main()
{
    // signal(SIGCHLD,chldhandler);
    pthread_t thread_id;
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, connectToClient, NULL);
    connectToData();
    while(1) {
        if(flag==1) {
            break;
        }
        len = sizeof(struct sockaddr_in);

        // Accept the data packet from client and verification
        dataconnfd[dataptr] = accept(datafd, (SA*)&clients[dataptr], &len);
        if (dataconnfd[dataptr] == -1) {
            if (errno == EWOULDBLOCK) {
                // printf("No pending connections; sleeping for one second.\n");
                sleep(1);
            } else {
                printf("error when accepting connection\n");
                exit(1);
            }
        } else {
            printf("Got a connection\n");
            char str[INET_ADDRSTRLEN];
            inet_ntop( AF_INET, &clients[dataptr].sin_addr, str, INET_ADDRSTRLEN );
            printf("%s\n",str);
            dataptr++;
        }
    }
    pthread_join(thread_id, NULL);
    pthread_create(&thread_id, NULL, readfromClient, NULL);

    sendiptoClient();
    NODE home = (NODE)malloc(sizeof(struct node));
    home->child = NULL;
    strcpy(home->name,"home");
    strcpy(home->type,"directory");
    home->parent = NULL;
    home->next = NULL;
    NODE cur = home;
    char choice[MAX_SIZE];
    while(1) {
        char choice[MAX_SIZE];
        copyToString(choice);
        if(strcmp(choice,"0")==0) {
            NODE temp = cur->child;
            while(temp!=NULL) {
                // printf("%s-",temp->name);
                write(connfd,temp->name,strlen(temp->name));
                write(connfd,"\n",1);
                temp = temp->next;
            }
            // printf("\n");
            write(connfd,"\0",1);
        } else if(strcmp(choice,"1")==0) {
            char dirname[MAX_SIZE];
            copyToString(dirname);

            NODE new = (NODE)malloc(sizeof(struct node));
            new->next = NULL;
            new->child = NULL;
            new->parent = cur;
            new->no_of_chunks = 0;
            strcpy(new->name,dirname);
            strcpy(new->type,"directory");
            if(cur->child==NULL) {
                cur->child = new;
            } else {
                // printf("-%s-",lastchild->name);
                NODE lastchild = cur->child;
                while(lastchild->next!=NULL) {
                    lastchild=lastchild->next;
                }
                lastchild->next = new;
            }
        } else if(strcmp(choice,"2")==0) {
            char *dirname = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(dirname);
            char *found;
            found = strsep(&dirname,"/");

            int flag=0;
            NODE temp_cur = cur;
            while(found!=NULL) {
                if(strcmp(found,"..")==0) {
                    temp_cur = temp_cur->parent;
                    found = strsep(&dirname,"/");
                    continue;
                } else if(strcmp(found,".")==0) {
                    found = strsep(&dirname,"/");
                    continue;
                }
                NODE temp=temp_cur->child;
                while(temp!=NULL) {
                    if(strcmp(temp->name,found)==0) {
                        if(strcmp(temp->type,"directory")!=0) {
                            flag=1;
                            break;
                        }
                        temp_cur = temp;
                        break;
                    }
                    temp=temp->next;
                }
                if(temp==NULL || flag==1) {
                    flag=1;
                    break;
                }
                found = strsep(&dirname,"/");

            }
            if(flag==0) {
                cur=temp_cur;
                write(connfd,"0\0",2);
            } else {
                write(connfd,"1\0",2);
            }

        } else if(strcmp(choice,"3")==0) {
            char *filename = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(filename);
            char *found;
            char found2[MAX_SIZE];
            found = strsep(&filename,"/");
            while(found!=NULL) {
                strcpy(found2,found);
                found = strsep(&filename,"/");
            }
            // strcpy(filename,found2);
            // printf("%s\n",found2);
            NODE temp = cur->child;
            int flag = 0;
            while(temp!=NULL) {
                if(strcmp(temp->name,found2)==0) {
                    flag=1;
                    break;
                }
                temp=temp->next;
            }
            if(flag==0) {
                write(connfd,"0\0",2);
            } else {
                write(connfd,"1\0",2);
                continue;
            }
            NODE new = (NODE)malloc(sizeof(struct node));
            new->next = NULL;
            new->child = NULL;
            new->parent = cur;
            strcpy(new->type,"file");

            strcpy(new->name,found2);
            char no_of_chunks[20];
            copyToString(no_of_chunks);
            int chunks = atoi(no_of_chunks);
            new->no_of_chunks = chunks;
            if(cur->child==NULL) {
                cur->child = new;
            } else {
                // printf("-%s-",lastchild->name);
                NODE lastchild = cur->child;
                while(lastchild->next!=NULL) {
                    lastchild=lastchild->next;
                }
                lastchild->next = new;
            }
            copyToString(found2);
            strcpy(new->loc_name, found2);
            printf("File Location: %s\n",found2);

        } else if(strcmp(choice, "4")==0) {
            char *source = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(source);
            char *destination = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(destination);
            printf("Source: %s\n",source);
            printf("Destination: %s\n",destination);
            char *found;
            char found2[MAX_SIZE];
            found = strsep(&source,"/");
            NODE temp_cur = cur;
            int flag=0;
            while(found!=NULL) {
                if(strcmp(found,"..")==0) {
                    temp_cur = temp_cur->parent;
                } else if(strcmp(found,".")==0) {

                } else {
                    NODE temp = temp_cur->child;
                    while(temp!=NULL) {
                        if(strcmp(temp->name,found)==0) {
                            temp_cur=temp;
                            break;
                        }
                        temp=temp->next;
                    }
                    if(temp==NULL) {
                        flag=1;
                        break;
                    }
                }
                found = strsep(&source,"/");
            }
            if(flag==1) {
                write(connfd,"1\0",2);
                continue;
            }
            // printf("%s\n",temp_cur->loc_name);
            NODE par = temp_cur->parent;
            if(strcmp(par->child->name,temp_cur->name)==0) {
                par->child = temp_cur->next;
                temp_cur->next = NULL;
            } else {
                NODE temp = par->child;
                NODE prev = temp;
                while(temp!=NULL) {
                    if(strcmp(temp->name,temp_cur->name)==0) {
                        prev->next = temp_cur->next;
                        temp_cur->next = NULL;
                        break;
                    }
                    prev = temp;
                    temp=temp->next;
                }
            }

            found = strsep(&destination,"/");
            NODE temp_cur2 = cur;
            int flag2=0;
            while(found!=NULL) {
                if(strcmp(found,"..")==0) {
                    temp_cur2 = temp_cur2->parent;
                } else if(strcmp(found,".")==0) {

                } else {
                    NODE temp = temp_cur2->child;
                    while(temp!=NULL) {
                        if(strcmp(temp->name,found)==0) {
                            temp_cur2=temp;
                            break;
                        }
                        temp=temp->next;
                    }
                    if(temp==NULL) {
                        strcpy(found2,found);
                        found = strsep(&destination,"/");
                        if(found!=NULL) {
                            flag=1;
                            break;
                        } else {
                            flag2=1;
                            break;
                        }

                    }
                }
                found = strsep(&destination,"/");
            }
            if(strcmp(temp_cur2->type,"file")==0) {
                flag=1;
            }
            if(flag==1) {
                write(connfd,"1\0",2);
                continue;
            }
            temp_cur->parent = temp_cur2;
            if(temp_cur2->child==NULL) {
                temp_cur2->child = temp_cur;
            } else {
                NODE temp = temp_cur2->child;
                while(temp->next!=NULL) {
                    temp=temp->next;
                }
                temp->next = temp_cur;
            }
            if(flag2==1) {
                strcpy(temp_cur->name,found2);
            }

            write(connfd,"0\0",2);
        } else if(strcmp(choice, "5")==0) {
            char *file = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(file);
            printf("File %s\n",file);
            NODE temp = cur->child;
            int flag=0;
            while(temp!=NULL) {
                if(strcmp(temp->name, file)==0) {
                    if(strcmp(temp->type, "file")==0) {
                        flag=1;
                        break;
                    }

                }
                temp=temp->next;
            }
            if(flag==0) {
                write(connfd,"0\0",2);
            } else {
                write(connfd,"1\0",2);
            }
            write(connfd,temp->loc_name,strlen(temp->loc_name)+1);
            char chunks[10];
            sprintf(chunks,"%d",temp->no_of_chunks);
            write(connfd,chunks,strlen(chunks)+1);
        } else if(strcmp(choice,"6")==0) {
            char *file = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(file);
            printf("File %s\n",file);
            NODE temp = cur->child;
            int flag=0;
            while(temp!=NULL) {
                if(strcmp(temp->name, file)==0) {
                    if(strcmp(temp->type, "file")==0) {
                        flag=1;
                        break;
                    }

                }
                temp=temp->next;
            }
            if(flag==0) {
                write(connfd,"0\0",2);
            } else {
                write(connfd,"1\0",2);
            }
            write(connfd,temp->loc_name,strlen(temp->loc_name)+1);
            char chunks[10];
            sprintf(chunks,"%d",temp->no_of_chunks);
            write(connfd,chunks,strlen(chunks)+1);

        } else if(strcmp(choice,"7")==0) {
            char *file = (char*)malloc(sizeof(char)*MAX_SIZE);
            copyToString(file);
            printf("File %s\n",file);
            NODE temp = cur->child;
            NODE prev = temp;
            int flag=0;
            if(temp==NULL) {
            } else if(strcmp(prev->name,file)==0) {
                cur->child = prev->next;
                prev->next = NULL;
                flag=1;
            } else {
                while(temp!=NULL) {
                    if(strcmp(temp->name, file)==0) {
                        if(strcmp(temp->type, "file")==0) {
                            flag=1;
                            prev->next = temp->next;
                            temp->next = NULL;
                            break;
                        }

                    }
                    prev=temp;
                    temp=temp->next;
                }
            }

            if(flag==0) {
                write(connfd,"0\0",2);
            } else {
                write(connfd,"1\0",2);
            }
            write(connfd,temp->loc_name,strlen(temp->loc_name)+1);
            char chunks[10];
            sprintf(chunks,"%d",temp->no_of_chunks);
            write(connfd,chunks,strlen(chunks)+1);
        }
    }

    // p = fork();
    // if(p==0) {
    //
    // }
    close(sockfd);

}
