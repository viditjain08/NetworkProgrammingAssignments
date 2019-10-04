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
#define MAX_SIZE 100
extern char **environ;



void checkHostName(int hostname)
{
    if (hostname == -1)
    {
        perror("gethostname");
        exit(1);
    }
}

// Returns host information corresponding to host name
void checkHostEntry(struct hostent * hostentry)
{
    if (hostentry == NULL)
    {
        perror("gethostbyname");
        exit(1);
    }
}

// Converts space-delimited IPv4 addresses
// to dotted-decimal format
void checkIPbuffer(char *IPbuffer)
{
    if (NULL == IPbuffer)
    {
        perror("inet_ntoa");
        exit(1);
    }
}

char* getip() {
    char hostbuffer[256];
    char *IPbuffer;
    struct hostent *host_entry;
    int hostname;

    // To retrieve hostname
    hostname = gethostname(hostbuffer, sizeof(hostbuffer));
    checkHostName(hostname);

    // To retrieve host information
    host_entry = gethostbyname(hostbuffer);
    checkHostEntry(host_entry);

    // To convert an Internet network
    // address into ASCII string
    IPbuffer = inet_ntoa(*((struct in_addr*)
                           host_entry->h_addr_list[0]));

    return IPbuffer;
}

typedef struct {
    char name[10];
    char ip[20];
} node;

int main() {

    FILE *fp;
    char buff[255];

    int n;
    fp = fopen("./config_file", "r");
    fscanf(fp, "%d\n", &n);
    printf("Number of Nodes: %d\n",n);
    node *list;
    list = (node*)malloc(sizeof(node)*n);
    for(int i=0;i<n;i++) {
        char temp1[255];
        memset(list[i].ip, '\0', 20*sizeof(char));
        fgets(temp1, sizeof(temp1), fp);
        int ptr=0;
        while(temp1[ptr]!=' ') {
            list[i].name[ptr] = temp1[ptr];
            ptr++;
        }
        list[i].name[ptr]='\0';
        ptr++;
        int j=0;
        while(temp1[ptr]!='\0' && temp1[ptr]!='\n') {
            list[i].ip[j] = temp1[ptr];
            j++;
            ptr++;
        }
        list[i].ip[j]='\0';
        printf("%s------>%s\n",list[i].name,list[i].ip);

    }
    fclose(fp);
    int idx=0;
    for(int i=0;i<n;i++) {
        if(strcpy(getip(),list[i].ip)==0) {
            idx=i;
            break;
        }
    }
    idx=1;

    // char buf[MAX_SIZE];
    // int length = read(0,buf,MAX_SIZE);
    // buf[length]='\0';
    // // printf("HOME : %s\n", getenv("HOME"));
	// system(buf);

    return 0;
}
