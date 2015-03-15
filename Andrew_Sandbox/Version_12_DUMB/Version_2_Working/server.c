/*
** listener.c -- a datagram sockets "server" demo = UDP
*
*  Version 2: Should be able to recieve a string and echo it back.
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define MYPORT "10130"    // the port users will be connecting to

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void)
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

	while(1) {

	printf("Listening...\n");

	if ((status = getaddrinfo(NULL,	// e.g www.example.com or IP 
		"10130", // Port Number
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
	return 0;
}
























