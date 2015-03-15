//
//  Vehicle.cpp
//
//
//  Created by Andrew Marshall on 2/23/15.
//  Refactored by Evan Hall on 3/12/15.
//  Repatterened by Andrew Marshall on 3/13/15
//
//

#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <iomanip>
#include <signal.h>
#include <sstream>
#include <string>
#include <memory>

#include "Resources.h"

#define MAXBUFLEN 100
#define MYPORT "10130"

int main(int argc, const char * argv[])
{
	
	
	
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int status;
	int numbytes;
	struct sockaddr_storage their_addr;
	char buf[MAXBUFLEN];
	socklen_t addr_len;
	char s[INET6_ADDRSTRLEN];
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // set to AF_INET to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE; // use my IP
	
	while(1)
	{
		
		printf("Listening...\n");
		
		if ((status = getaddrinfo(NULL,	// e.g www.example.com or IP
								  MYPORT, // Port Number
								  &hints, // points to a struct with info we have already filled in
								  &servinfo)) != 0)	// servinfo: A linked list of results
		{
			fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
			return 1;
		}
		
		// loop through all the results and bind to the first we can
		for(p = servinfo; p != NULL; p = p->ai_next)
		{
			if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
				perror("listener: socket");
				continue;
			}
			
			if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
				close(sockfd);
				perror("listener: bind");
				continue;
			}
			
			break;
			
			
			if (p == NULL) {
				fprintf(stderr, "listener: failed to bind socket\n");
				return 2;
			}
		}
		
		freeaddrinfo(servinfo); // Call this when we are done with the struct "servinfo"
		
		printf("listener: waiting to recvfrom...\n");
		
		addr_len = sizeof their_addr;
		
		if ((numbytes = recvfrom(sockfd, buf, MAXBUFLEN-1 , 0,
								 (struct sockaddr *)&their_addr, &addr_len)) == -1) {
			perror("recvfrom");
			exit(1);
		}
		
		printf("listener: got packet from %s\n",
			   inet_ntop(their_addr.ss_family,
						 get_in_addr((struct sockaddr *)&their_addr),
						 s, sizeof s));
		
		printf("listener: packet is %d bytes long\n", numbytes);
		buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
		//printf("listener: packet is %d bytes long\n", numbytes);
		//buf[numbytes] = '\0';
		//printf("listener: packet contains \"%s\"\n", buf);
		
		// Adding code to try and send an echo, keep your fingers crossed
		
		printf("listener: Sending echo...\n");
		
		buf[2] = '%';
		//    buf[numbytes] = '\0';
		
		if( sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&their_addr, addr_len) == -1)
		{
			perror("listener: sendto error");
			exit(1);
		}
		
		printf("listener: packet is %d bytes long\n", numbytes);
		//    buf[numbytes] = '\0';
		printf("listener: packet contains \"%s\"\n", buf);
		
		//    freeaddrinfo(servinfo); // Call this when we are done with the struct "servinfo"
		close(sockfd);
		
	}

	
	exit(1);
	
	return 0;
}



































