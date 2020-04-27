// compile as -o ping 
// run as sudo ./ping <hostname> 

#include <stdio.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <arpa/inet.h> 
#include <netdb.h> 
#include <unistd.h> 
#include <string.h> 
#include <stdlib.h> 
#include <netinet/ip_icmp.h> 
#include <time.h> 
#include <fcntl.h> 
#include <signal.h> 
#include <time.h> 

#define PING_PKT_S 64 

#define PORT_NO 0 

#define PING_SLEEP_RATE 1000000

#define RECV_TIMEOUT 1 

int pingloop=1; 


struct ping_pkt 
{ 
	struct icmphdr hdr; 
	char msg[PING_PKT_S-sizeof(struct icmphdr)]; 
}; 

unsigned short checksum(void *b, int len) 
{ unsigned short *buf = b; 
	unsigned int sum=0; 
	unsigned short result; 

	for ( sum = 0; len > 1; len -= 2 ) 
		sum += *buf++; 
	if ( len == 1 ) 
		sum += *(unsigned char*)buf; 
	sum = (sum >> 16) + (sum & 0xFFFF); 
	sum += (sum >> 16); 
	result = ~sum; 
	return result; 
} 


void intHandler(int dummy) 
{ 
	pingloop=0; 
} 


void send_ping(int ping_sockfd, struct sockaddr_in *ping_addr) 
{ 
	int ttl_val=64, msg_count=0, i, addr_len, flag=1, msg_received_count=0; 
	
	struct ping_pkt pckt; 
	struct sockaddr_in r_addr; 
	struct timespec time_start, time_end, tfs, tfe; 
	long double rtt_msec=0, total_msec=0; 
	struct timeval tv_out; 
	tv_out.tv_sec = RECV_TIMEOUT; 
	tv_out.tv_usec = 0; 

	clock_gettime(CLOCK_MONOTONIC, &tfs); 

	
	if (setsockopt(ping_sockfd, SOL_IP, IP_TTL, &ttl_val, sizeof(ttl_val)) != 0) 
	{ 
		printf("\nSetting socket options to TTL failed!\n"); 
		return; 
	} 
	else
	{ 
		printf("\nSocket set to TTL..\n"); 
	} 
	setsockopt(ping_sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv_out, sizeof tv_out); 

	while(pingloop) 
	{ 
		flag=1; 
	
		bzero(&pckt, sizeof(pckt)); 
		

		addr_len=sizeof(r_addr); 
		int bytesrevcv = 0;
		if ( (bytesrevcv = recvfrom(ping_sockfd, &pckt, sizeof(pckt), 0, (struct sockaddr*)&r_addr, &addr_len)) <= 0 && msg_count>1) 
		{ //printf("\nPacket receive failed!\n"); 
		} 

		else
		{ 
			clock_gettime(CLOCK_MONOTONIC, &time_end); 
			
			double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0 ;
			rtt_msec = (time_end.tv_sec- time_start.tv_sec) * 1000.0 + timeElapsed; 
			char ch[1] = "c";
			if(flag) 
			{ 
				if((pckt.hdr.type ==69 && pckt.hdr.code==0)) 
				{
					printf("%d bytes from %s ICMP TYPE : %d ICMP CODE: %d icmp_seq=%d ttl=%d rtt = %Lf ms.\n",bytesrevcv, inet_ntoa(r_addr.sin_addr),pckt.hdr.type,pckt.hdr.code, msg_received_count,	ttl_val, rtt_msec); 
					msg_received_count++; 
				} 
				
			} 
		}	 
	} 
	clock_gettime(CLOCK_MONOTONIC, &tfe); 
	double timeElapsed = ((double)(tfe.tv_nsec - tfs.tv_nsec))/1000000.0; 
	
	total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+ 	timeElapsed ;
					
	printf("\n===%s ping statistics===\n", "c"); 
	printf("\n%d packets received. Total time: %Lf ms.\n\n",msg_received_count, 		total_msec); 
} 

int main(int argc, char *argv[]) 
{ 
	int sockfd; 
	struct sockaddr_in addr_con; 
	int addrlen = sizeof(addr_con); 
	char net_buf[NI_MAXHOST]; 
	//socket() 
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); 
	if(sockfd<0) 
	{ 
		printf("\nSocket file descriptor not received!!\n"); 
		return 0; 
	} 
	else
		printf("\nSocket file descriptor %d received\n", sockfd); 

	signal(SIGINT, intHandler);

	send_ping(sockfd, &addr_con); 
	
	return 0; 
} 
