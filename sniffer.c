#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <errno.h>

int main(){
   struct sockaddr sock_adder;
   struct sockaddr_in src_addr, dest_addr;
   struct packet_mreq mr;
   unsigned int addr_len = sizeof(sock_adder);
   int sock = -1;
   
   sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP); //build raw socket
   if(sock == -1){
   	printf("failed to open socket\n");
   	return -1;
   	}
   //set promiscuous mode	
   mr.mr_type = PACKET_MR_PROMISC;                           
   setsockopt(sock, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr));
   //listen to the ICMP messages   
   int i = 1;             
   while(1){
   	char buf[IP_MAXPACKET];
   	if(recvfrom(sock, buf, IP_MAXPACKET, 0, &sock_adder, &addr_len) > 0){  //recive packet
            struct iphdr *iph = (struct iphdr *) buf;  //translate to ip headaer
            int iph_len = iph->ihl*4;
            struct icmphdr *icmph = (struct icmphdr *)(buf + iph_len); //translate to icmp header
            memset(&src_addr, 0, sizeof(src_addr));      //extract values
            src_addr.sin_addr.s_addr = iph->saddr;
            memset(&dest_addr, 0, sizeof(dest_addr));
            dest_addr.sin_addr.s_addr = iph->daddr;
            printf("Packet number:%d\n",i++);
            printf("TYPE: %d\n", (unsigned int)(icmph->type)); 
            printf("CODE: %d\n", (unsigned int)(icmph->code));
            printf("SRC IP: %s\n", inet_ntoa(src_addr.sin_addr));
            printf("DEST IP: %s\n\n", inet_ntoa(dest_addr.sin_addr));
   	}
   	bzero(buf,IP_MAXPACKET); //reset buffer
   }
   close(sock);
   return 0;
}

