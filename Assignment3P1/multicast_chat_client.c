#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <time.h>
#include <sys/shm.h>
#define KEY 515
#define NO_OF_COURSES 100

// size 58
struct course
{
    char name[50];
    int recv;
    int send;
}typedef course;
int main(int argc, char *argv[])
{
    if(argc!=3)
    {
        printf("Invalid arguments, Provide group IP and port\n");
        exit(0);
    }
    pid_t pid;
    pid=fork();
    if(pid==0)
    {
        int shmid;
        course *data;
        if ((shmid = shmget (KEY, 58*NO_OF_COURSES, 0644 | IPC_CREAT)) == -1)
        {
            perror ("shmget: shmget failed");
            exit (1);
        }
        data = shmat (shmid, (void *) 0, 0);

        if (data == (course *) (-1))
            perror ("shmat");
        struct sockaddr_in addr;
        struct ip_mreq mreq;
        int sock = socket(AF_INET, SOCK_DGRAM, 0);
        bzero((char *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(atoi(argv[2]));
        int addrlen = sizeof(addr);
        if(bind(sock, (struct sockaddr *) &addr, sizeof(addr)) < 0)
        {
            perror("bind");
            exit(0);
        }
        mreq.imr_multiaddr.s_addr = inet_addr(argv[1]);
        mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if(setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0)
        {
            perror("setsockopt mreq");
            exit(0);
        }
        while(1)
        {
            char msg[100];
            char name[50];
            recvfrom(sock, msg, sizeof(msg), 0, (struct sockaddr *) &addr, &addrlen);
            printf("Message Recieved:\n %s\n ",msg);
            if(msg[strlen(msg)-1]=='?')
            {
                strcpy(name,strtok(msg," "));
                int i;
                int pos=-1;
                for(i=0;i<NO_OF_COURSES;i++)
                {
                    if(data[i].name[0]=='\0')
                    {
                        pos=i;
                        break;
                    }
                    else if(strcmp(data[i].name,name)==0)
                    {
                        data[i].send=1;
                        break;
                    }
                }
                if(pos!=-1)
                {
                    strcpy(data[pos].name,name);
                    data[pos].send=1;
                }
            }
            else
            {
                strcpy(name,strtok(msg," "));
                int i;
                int pos=-1;
                for(i=0;i<NO_OF_COURSES;i++)
                {
                    if(data[i].name[0]=='\0')
                    {
                        pos=i;
                        break;
                    }
                    else if(strcmp(data[i].name,name)==0)
                    {
                        data[i].recv=1;
                        break;
                    }
                }
                if(pos!=-1)
                {
                    strcpy(data[pos].name,name);
                    data[pos].recv=1;
                }
            }

        }

    }
    else
    {
        int shmid;
        course *data;
        if ((shmid = shmget (KEY, 58*NO_OF_COURSES, 0644 | IPC_CREAT)) == -1)
        {
            perror ("shmget: shmget failed");
            exit (1);
        }
        data = shmat (shmid, (void *) 0, 0);

        if (data == (course *) (-1))
            perror ("shmat");

        struct sockaddr_in addr;
        int addrlen, sock, cnt;
        struct ip_mreq mreq;
        char msg[100];
        sock = socket(AF_INET, SOCK_DGRAM, 0);
        bzero((char *)&addr, sizeof(addr));
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port = htons(atoi(argv[2]));
        addrlen = sizeof(addr);
        addr.sin_addr.s_addr = inet_addr(argv[1]);
        while(1)
        {
            printf("Press 1 for asking when is the test\n");
            printf("Press 2 for telling people when is the test\n");
            int choice;
            scanf("%d",&choice);
            if(choice==1)
            {
                printf("Which course?\n");
                char name[50];
                scanf("%s",name);
                int i;
                int flag=0;
                for(i=0;i<NO_OF_COURSES;i++)
                {
                    if(data[i].name[0]=='\0')
                        break;
                    if(strcmp(data[i].name,name)==0)
                    {
                        if(data[i].recv==1)
                        {
                            flag=1;
                            break;
                        }
                    }
                }
                if(flag==1)
                {
                    printf("Somebody has already sent the answer before\n");
                    printf("Do you want to ask again? y/n\n");
                    char ch[2];
                    scanf("%s",ch);
                    if(strcmp(ch,"y")==0)
                        flag=0;

                }
                if(flag==0)
                {
                    strcpy(msg,name);
                    strcat(msg," test date?");
                    sendto(sock,msg,sizeof(msg),0,(struct sockaddr *)&addr,addrlen);
                }
            }
            else if(choice==2)
            {
                printf("Which course?\n");
                char name[50];
                scanf("%s",name);
                int i,flag=0;
                for(i=0;i<NO_OF_COURSES;i++)
                {
                    if(data[i].name[0]=='\0')
                        break;
                    if(strcmp(data[i].name,name)==0)
                    {
                        if(data[i].send==1)
                        {
                            flag=1;
                            break;
                        }
                    }
                }

                if(flag==0)
                {
                    printf("No one has asked for this course\n");
                    printf("Do you still want to tell people? y/n\n");
                    char ch[2];
                    scanf("%s",ch);
                    if(strcmp(ch,"y")==0)
                    {
                        flag=1;
                    }
                }
                if(flag==1)
                {
                    printf("When is the test?\n");
                    char ans[20];
                    scanf("%s",ans);
                    strcpy(msg,name);
                    strcat(msg," ");
                    strcat(msg,ans);
                    sendto(sock,msg,sizeof(msg),0,(struct sockaddr *)&addr,addrlen);

                }
            }
            else
            {
                printf("Invalid Choice\n");
                exit(0);
            }


        }
    }
}
