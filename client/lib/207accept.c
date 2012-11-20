#include "207layer.h"

#include "connect207.h"
#include <errno.h>
#define MAX 512

int cmpe207_accept(int sockfd, struct sockaddr_in *addr, socklen_t * addrlen)
{
//error checking: check sockfd IN USE, in LISTEN state
	if(!CB[sockfd].sock_in_use)
		die("accept(): socket not in use.");
	if(!check_state(sockfd, "LISTEN"))
		die("accept(): not in LISTEN state.");

//create slave socket
	int ssockfd;
	char self_addr[INET_ADDRSTRLEN];
	char * str;	
	struct sockaddr_in s_client;

//create slave socket
	ssockfd = cmpe207_socket(CMPE207_FAM, CMPE207_SOC, CMPE207_PROC);
	check_for_error(ssockfd, "accept(): couldn't create slave socket");
	close(CB[ssockfd].sockfd_udp);

	CB[ssockfd].sock_struct_UDP.sin_family = CB[sockfd].sock_struct_UDP.sin_family;
	CB[ssockfd].sock_struct_UDP.sin_port = CB[sockfd].sock_struct_UDP.sin_port;
	CB[ssockfd].sock_struct_UDP.sin_addr = CB[sockfd].sock_struct_UDP.sin_addr;
	CB[ssockfd].tcp_state = CB[sockfd].tcp_state;
	CB[ssockfd].sockfd_udp = CB[sockfd].sockfd_udp;
	//printf("fd: %s \n", inet_ntop(AF_INET, &(CB[ssockfd].sock_struct_UDP.sin_addr), str, INET_ADDRSTRLEN));

	int rv;
	char buf[MAX];

	int slen = sizeof s_client;
//init packet-header structure
	struct packet_header pTcpH, send_packet;
	CB[ssockfd].pTcpH = &pTcpH;

//3 way handshake
	while(!check_state(ssockfd, "ESTABLISHED") && check_state(sockfd, "LISTEN"))	
	{
		rv = recvfrom (CB[sockfd].sockfd_udp, buf, MAX, 0, (struct sockaddr*)&s_client, &slen);

		if(rv <= 0)
		{
			printf ("Error: recvfrom() %s\n",strerror(errno));
		}

		printf ("tcp_header_extract_from_recv_packet()\n");
		rv = tcp_header_extract_from_recv_packet(ssockfd, buf);
		if(rv != 0)
		{
			printf("Error: Returned from tcp_header_extract_from_recv_packet()\n");
			return rv;
		}

		CB[ssockfd].tcp_state = get_tcp_state (CB[ssockfd].tcp_state, *CB[ssockfd].pTcpH, 0);
		
		printf("state: %s \n sockfd: %d\n ssockfd: %d\n", (char*) get_state_name(CB[ssockfd].tcp_state), sockfd, ssockfd);

		if(!check_state(ssockfd, "SYN_RCVD"))
		{
			bzero (&send_packet, sizeof (send_packet));
			CB[ssockfd].pTcpH->syn_flag = 1;
			CB[ssockfd].pTcpH->ack_flag = 1;
			memcpy (&buf, &send_packet, sizeof (send_packet));
			rv = sendto (CB[sockfd].sockfd_udp, buf, MAX, 0, (struct sockaddr*)&s_client, slen);
			check_for_error (rv, "sendto()");
		}

	}

	return ssockfd;
}
/*
printf("state: %s \n", (char*) get_state_name(CB[sockfd].tcp_state));
	printf("sstate: %s \n", (char*) get_state_name(CB[ssockfd].tcp_state));
*/

