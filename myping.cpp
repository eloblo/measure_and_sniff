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
#include <errno.h>
#include <sys/time.h> // gettimeofday()
#include <time.h>

 // IPv4 header len without options
#define IP4_HDRLEN 20

// ICMP header len for echo req
#define ICMP_HDRLEN 8 


// Checksum algo
unsigned short calculate_checksum(unsigned short * paddress, int len);

#define SOURCE_IP "10.0.2.7"
#define DESTINATION_IP "172.67.70.92"

int main ()
{
    struct icmp icmphdr; // ICMP-header
    char data[IP_MAXPACKET] = "This is the ping.\n";

    int datalen = strlen(data) + 1;

    //===================
    // ICMP header
    //===================

    // Message Type (8 bits): ICMP_ECHO_REQUEST
    icmphdr.icmp_type = ICMP_ECHO;

    // Message Code (8 bits): echo request
    icmphdr.icmp_code = 0;

    // Identifier (16 bits): some number to trace the response.
    // It will be copied to the response packet and used to map response to the request sent earlier.
    // Thus, it serves as a Transaction-ID when we need to make "ping"
    icmphdr.icmp_id = 18; 

    // Sequence Number (16 bits): starts at 0
    icmphdr.icmp_seq = 0;

    // ICMP header checksum (16 bits): set to 0 not to include into checksum calculation
    icmphdr.icmp_cksum = 0;

    // Combine the packet 
    char packet[ICMP_HDRLEN + datalen];

    // add ICMP header
    memcpy (packet , &icmphdr, ICMP_HDRLEN);

    // After ICMP header, add the ICMP data.
    memcpy (packet + ICMP_HDRLEN, data, datalen);

    // Calculate the ICMP header checksum
    icmphdr.icmp_cksum = calculate_checksum((unsigned short *) (packet), ICMP_HDRLEN + datalen);
    memcpy ((packet), &icmphdr, ICMP_HDRLEN);

    struct sockaddr_in dest_in, recv_in;
    memset (&dest_in, 0, sizeof (struct sockaddr_in));
    dest_in.sin_family = AF_INET;

    // set the relavant ip addres
    dest_in.sin_addr.s_addr = inet_addr(DESTINATION_IP);
    

    // Create raw socket for IP-RAW (make IP-header by yourself)
    int sock = -1;
    if ((sock = socket (AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) 
    {
        fprintf (stderr, "socket() failed with error: %d", errno);
        fprintf (stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
        return -1;
    }

    // Send the packet using sendto() for sending datagrams.
    

    
    struct timespec time_start, time_end, tfs, tfe; //structs to hold specific time
    long double total_msec=0;  
    clock_gettime(CLOCK_MONOTONIC, &tfs);        //set starting time
    clock_gettime(CLOCK_MONOTONIC, &time_start);
    
    char buffer[IP_MAXPACKET];
    bzero(buffer, IP_MAXPACKET);
    unsigned int len = sizeof (recv_in);
    if (sendto (sock, packet, ICMP_HDRLEN + datalen, 0, (struct sockaddr *) &dest_in, sizeof (dest_in)) == -1)  
    {
        fprintf (stderr, "sendto() failed with error: %d", errno);
        return -1;
    }
    while(1){  //filter replies
    	if(recvfrom(sock, buffer, IP_MAXPACKET, 0, (struct sockaddr *) &recv_in, &len) <= 0){ //recive the reply
            printf("failed to recive\n");
    	    return -1;
    	}
    	struct sockaddr_in dest_addr, src_addr;       //addrs for the dest and src ip's
    	struct iphdr *iph = (struct iphdr *) buffer;  //translate the buffer to ip header struct
    	int iph_len = iph->ihl*4;
        struct icmphdr *icmph = (struct icmphdr *)(buffer + iph_len);  //translate the buffer to icmp header struct
    	memset(&src_addr, 0, sizeof(src_addr));       //extract the ips
        src_addr.sin_addr.s_addr = iph->saddr;
        memset(&dest_addr, 0, sizeof(dest_addr));
        dest_addr.sin_addr.s_addr = iph->daddr;
        char src_check[17];                           //convert ip to string
        char dest_check[17];
        memcpy(dest_check, inet_ntoa(dest_addr.sin_addr), 17);
        memcpy(src_check, inet_ntoa(src_addr.sin_addr), 17);
        //filter for relevant dest and src ips' and the type of reply
        if(strcmp(dest_check, SOURCE_IP) == 0 && strcmp(src_check, DESTINATION_IP) == 0 && icmph->type == 0){ 
            break;
        } 
         
    }
    clock_gettime(CLOCK_MONOTONIC, &time_end);          //set ending time 
    clock_gettime(CLOCK_MONOTONIC, &tfe); 
    double timeElapsed = ((double)(time_end.tv_nsec - time_start.tv_nsec))/1000000.0;  //calculate nanoseconds to miliseconds      
    total_msec = (tfe.tv_sec-tfs.tv_sec)*1000.0+timeElapsed;  //add the seconds
    printf("Total time: %Lf ms.\n", total_msec); 
                  

  // Close the raw socket descriptor.
  close(sock);

  return 0;
}

// Compute checksum (RFC 1071).
unsigned short calculate_checksum(unsigned short * paddress, int len)
{
	int nleft = len;
	int sum = 0;
	unsigned short * w = paddress;
	unsigned short answer = 0;

	while (nleft > 1)
	{
		sum += *w++;
		nleft -= 2;
	}

	if (nleft == 1)
	{
		*((unsigned char *)&answer) = *((unsigned char *)w);
		sum += answer;
	}

	// add back carry outs from top 16 bits to low 16 bits
	sum = (sum >> 16) + (sum & 0xffff); // add hi 16 to low 16
	sum += (sum >> 16);                 // add carry
	answer = ~sum;                      // truncate to 16 bits

	return answer;
}




