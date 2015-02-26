/*
 *	Comp5360: Project 1
 *
 *	File: UDP_Component.cpp
 *	Author: Andrew K. Marshall (akm0012)
 *	Group ID: 15
 *	Date: 2/25/15
 *	Version: 0.1
 *	Version Notes: Testing
 */

#include "UDP_Component.h"

#define DEBUG 1	// Used for debugging 1 = ON, 0 = OFF

using namespace std;

// Used to determine if we are using IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	//else
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void send_packet(string hostname_to_send, string port_to_send, packet_to_send packet_out)
{
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int rv;
	int numbytes_tx;
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	
	memset(&hints, 0, sizeof hints);	// put 0's in all the mem space for hints (clearing hints)
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;
	
	if ((rv = getaddrinfo(hostname_to_send.c_str(), port_to_send.c_str(), &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		exit(1);
	}
	
	// loop through all the results and make a socket
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
							 p->ai_protocol)) == -1)
		{
			perror("Error: socket");
			continue;
		}
		
		break;
	}
	
	if (p == NULL) {
		fprintf(stderr, "Error: failed to bind socket\n");
		exit(1);
	}
	
	// Here is where we send the data
	if ((numbytes_tx = sendto(sockfd, (char *)&packet_out, sizeof(packet_out), 0,
							  p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("Error: sendto");
		exit(1);
	}
	
	if (DEBUG) {
		printf("Sent %d bytes to %s\n", numbytes_tx, hostname_to_send.c_str());
	}
	
	freeaddrinfo(servinfo);
	close(sockfd);
}


void start_receiving(string port_in)
{
	// Variables
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int status;
	int numbytes;
	char *my_port;
	
	//struct sockaddr_storage their_addr;
	struct sockaddr_in their_addr;
	socklen_t addr_len;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	// set to AF_INIT to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP
	
	
	if (DEBUG) {
		printf("DEBUG: Port number: %s\n", port_in.c_str());
	}
	
	printf("Starting Server... to stop, press 'Ctrl + c'\n");
	
	while(1)
	{
		// 1. getaddrinfo
		if ((status = getaddrinfo(NULL, port_in.c_str(),
								  &hints, 	// points to a struct with info we have already filled in
								  &servinfo)) != 0)	// servinfo: A linked list of results
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
			exit(1);
		}
		
		// Loop through all the results and bind to the first we can
		for (p = servinfo; p != NULL; p = p->ai_next)
		{
			// 2. socket
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
			{
				perror("Socket Error!");
				continue;
			}
			
			// 3. bind
			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1)
			{
				close(sockfd);
				perror("Bind Error!");
				continue;
			}
			
			break;
		}
		
		if (p == NULL)
		{
			fprintf(stderr, "Failed to bind socket\n");
			exit(1);
		}
		
		
		freeaddrinfo(servinfo); 	// Call this when we are done with the struct "servinfo"
		
		if (DEBUG) {
			printf("Binding complete, waiting to recvfrom...\n");
		}
		
		addr_len = sizeof their_addr;
		
		tx_packet packet_in;
		
		// 4. recvfrom
		// MAX_PACKET_LEN -1: To make room for '\0'
		if ((numbytes = recvfrom(sockfd, (char *) &packet_in, MAX_PACKET_LEN - 1, 0,
								 (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}
		
		
		printf("Packet Received! It contained: %d bytes.\n", numbytes);
		
		// Print out Contents
//		if (DEBUG) {
//			printf("\n----- Packet In -----\n");
//			printf("packet_in.short_1: %X\n", ntohs(packet_in.short_1));
//			printf("packet_in.char_1: %d\n", packet_in.char_1);
//			printf("packet_in.short_2: %d\n", ntohs(packet_in.short_2));
//			
//		}
		
//		// 5. Send Error message to client
//		if (sendto(sockfd, (char *)&tx_err_len, sizeof(tx_err_len),
//				   0, (struct sockaddr *)&their_addr, addr_len) == -1)
//		{
//			perror("sendto error");
//			exit(1);
//		}
		
		close(sockfd);
	}
}




int main(int argc, char *argv[])
{

	if (strcmp(argv[1], "1")) {
		// The packet we will send
		tx_packet packet_out;
		
		// Get the Packet Ready to Send
//		packet_out.short_1 = htons(0x1234);
//		packet_out.char_1 = 2;
//		packet_out.short_2 = htons(10025);
		
		
//		if (DEBUG) {
//			printf("\n----- Packet Out -----\n");
//			printf("packet_out.short_1: %X\n", ntohs(packet_out.short_1));
//			printf("packet_out.char_1: %d\n", packet_out.char_1);
//			printf("packet_out.short_2: %d\n", ntohs(packet_out.short_2));
//			
//		}
		
		send_packet("ubuntu", "10025", packet_out);
	}
	
	else if (strcmp(argv[1], "2")) {
		
		start_receiving("10025");
		
	}
	
	else {
		cout << "Params should be 1 to send and 2 to receive.\n";
	}
	
	return 0;
}
