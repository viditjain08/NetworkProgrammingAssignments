#include<stdlib.h>
#include<unistd.h>
#include<stdio.h>
#include<sys/wait.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include<time.h>

typedef struct mesg_buffer {
    long mesg_type;
    int mesg_text;
} message;

int main(int argc, char **argv) {

    srand(time(0));
    key_t key,key2;
    int msgid,msgid2;

    // ftok to generate unique key
    key = ftok("./Makefile", 'B');
    key2 = ftok("./voting.c", 'B');
    msgid = msgget(key, IPC_CREAT | 0644);
    msgctl(msgid, IPC_RMID, NULL);
    msgid = msgget(key, IPC_CREAT | 0644);

    msgid2 = msgget(key2, IPC_CREAT | 0644);
    msgctl(msgid2, IPC_RMID, NULL);
    msgid2 = msgget(key2, IPC_CREAT | 0644);

    // printf("%d %d\n",msgid,msgid2);
    int n = atoi(argv[1]);
    printf("Number of Child Processes: %d\n",n);
    pid_t pid=-1;
    for(int i=0;i<n;i++) {
        if(pid!=0) {
            pid=fork();
        }
    }
    while(1) {
        if(pid==0) {
                // printf("Child Process %d %d\n",getpid(),msgid);
            srand(10*getpid()+time(0));

            message m;
            message temp;
            msgrcv(msgid2, &temp, sizeof(temp), 0, 0);

            m.mesg_type = 1;
            int toss = rand()%2;
            printf("Child Process %d votes for %d\n",getpid(),toss);
            m.mesg_text = toss;
            // msgsnd to send message
            msgsnd(msgid, &m, sizeof(m), 0);

        } else {
            sleep(1);
            printf("-------------------------------------------------------------------------------\n");
            printf("Voting Starts.............\n");
            for(int i=0;i<n;i++) {
                message temp;
                temp.mesg_type = 1;
                temp.mesg_text = -1;
                msgsnd(msgid2, &temp, sizeof(temp), 0);
            }
            message m;
            int c0=0,c1=0;
            while((c0+c1)!=n) {
                msgrcv(msgid, &m, sizeof(m), 0, 0);
                if(m.mesg_text==0) {
                    c0++;
                } else {
                    c1++;
                }
            }
            if(c0==c1) {
                printf("It's a draw with %d votes for both 0s and 1s\n",c0);
            } else {
                printf("%d wins with %d vote(s) for 0 and %d vote(s) for 1\n",c1>c0,c0,c1);
            }
        }
    }




    return 0;
}
