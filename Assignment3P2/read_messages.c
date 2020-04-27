// raw_sock.c
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#include <net/ethernet.h> /* the L2 protocols */
#include <netdb.h>
#include<netinet/tcp.h>
#include<netinet/udp.h>
#include <unistd.h>
#include <fcntl.h>

int main() {
    // Structs that contain source IP addresses
    struct sockaddr_in source_socket_address, dest_socket_address;

    int packet_size;
    int fd = open("packet_sniffer",O_WRONLY | O_CREAT);
    dup2(fd, 1);

    // Allocate string buffer to hold incoming packet data
    unsigned char *buffer = (unsigned char *)malloc(65536);
    // Open the raw socket
    int sock = socket (AF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
    //setsockopt(sock_raw , SOL_SOCKET , SO_BINDTODEVICE , &quot;eth0&quot; , strlen(&quot;eth0&quot;)+ 1 );
    if(sock == -1)
    {
        //socket creation failed, may be because of non-root privileges
        perror("Failed to create socket");
        exit(1);
    }
    while(1) {
      // recvfrom is used to read data from a socket
        packet_size = recvfrom(sock , buffer , 65536 , 0 , NULL, NULL);
        if (packet_size == -1) {
        printf("Failed to get packets\n");
        return 1;
        }
        struct iphdr *ip_packet = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        struct ethhdr *eth = (struct ethhdr *)buffer;
        memset(&source_socket_address, 0, sizeof(source_socket_address));
        source_socket_address.sin_addr.s_addr = ip_packet->saddr;
        memset(&dest_socket_address, 0, sizeof(dest_socket_address));
        dest_socket_address.sin_addr.s_addr = ip_packet->daddr;

        // if(ip_packet->protocol!=6) continue;
        printf("Incoming Packet: \n");

        printf("Source MAC      : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_source[0] , eth->h_source[1] , eth->h_source[2] , eth->h_source[3] , eth->h_source[4] , eth->h_source[5] );
        printf("Destination MAC : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X \n", eth->h_dest[0] , eth->h_dest[1] , eth->h_dest[2] , eth->h_dest[3] , eth->h_dest[4] , eth->h_dest[5] );

        printf("Source Address: %s\n", (char *)inet_ntoa(source_socket_address.sin_addr));
        printf("Destination Address: %s\n", (char *)inet_ntoa(dest_socket_address.sin_addr));

        printf("Packet Size (bytes): %d\n",ntohs(ip_packet->tot_len));

        printf("TTL      : %d\n",(unsigned int)ip_packet->ttl);

        if(ip_packet->protocol==6) {
            int iphdrlen = (ip_packet->ihl)*4;
        	struct tcphdr *tcph = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
            printf("Source Port      : %u\n",ntohs(tcph->source));
            printf("Destination Port : %u\n",ntohs(tcph->dest));

        } else if(ip_packet->protocol==17) {
            int iphdrlen = (ip_packet->ihl)*4;

        	struct udphdr *udph = (struct udphdr*)(buffer + iphdrlen  + sizeof(struct ethhdr));
            printf("Source Port      : %u\n",ntohs(udph->source));
            printf("Destination Port : %u\n",ntohs(udph->dest));
        }

        if(getprotobynumber(ip_packet->protocol)!=NULL)
            printf("Protocol: %s\n",getprotobynumber(ip_packet->protocol)->p_name);
        else
            printf("Protocol Number: %d\n",ip_packet->protocol);

        if(ip_packet->protocol==6) {
            int iphdrlen = (ip_packet->ihl)*4;
            struct tcphdr *tcph = (struct tcphdr*)(buffer + iphdrlen + sizeof(struct ethhdr));
            printf("TCP Window Size: %d\n",ntohs(tcph->window));
            printf("TCP SYN: %d\n",(unsigned int)tcph->syn);
            printf("TCP FIN: %d\n",(unsigned int)tcph->fin);
            printf("TCP RST: %d\n",(unsigned int)tcph->rst);
            printf("TCP ACK: %d\n",ntohs(tcph->ack_seq));

        }
      // printf("Identification: %d\n\n", ntohs(ip_packet->id));
        printf("----------------------------------------------------------\n");
    }

    return 0;
}
