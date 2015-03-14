/*
** talker.c -- a datagram "client" demo
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

#define MAXBUFLEN 100
#define SERVERPORT "4950"    // the port users will be connecting to

int main(int argc, char *argv[])
{
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    int numbytes_tx;
	int numbytes_rx;
	char buf[MAXBUFLEN];
	struct sockaddr_storage their_addr;
	socklen_t addr_len;
	

    if (argc != 3) {
        fprintf(stderr,"usage: talker hostname message\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;

    if ((rv = getaddrinfo(argv[1], SERVERPORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and make a socket
    for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) 
		{
            perror("talker: socket");
            continue;
		}
/*
		//Add code to bind as well
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) 
		{
			close(sockfd);
			perror("talker: bind");
			continue;
		}        

*/
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        return 2;
    }

    if ((numbytes_tx = sendto(sockfd, argv[2], strlen(argv[2]), 0,
             p->ai_addr, p->ai_addrlen)) == -1) {
        perror("talker: sendto");
        exit(1);
    }

//    freeaddrinfo(servinfo);

    printf("talker: sent %d bytes to %s\n", numbytes_tx, argv[1]);
    
	//Adding code to try and recieve an echo

	printf("talker: waiting for conformation echo...\n");

	addr_len = sizeof their_addr;

	if ((numbytes_rx = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
		(struct sockaddr *)&their_addr, &addr_len)) == -1)
	//if ((numbytes_rx = recvfrom(sockfd, buf, MAXBUFLEN-1, 0, 
	//	p->ai_addr, p->ai_addrlen)) == -1)
	{
		perror("recvfrom");
		exit(1);
	}

	printf("talker: received echo, packet is %d bytes long.\n", numbytes_rx);

	buf[numbytes_rx] = '\0';
	
	printf("talker: echo packet contains: \"%s\"\n", buf);

    freeaddrinfo(servinfo);
	close(sockfd);

    return 0;
}


































