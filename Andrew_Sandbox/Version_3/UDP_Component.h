//
//  UDP_Component.h
//  
//
//  Created by Andrew Marshall on 2/25/15.
//
//

#ifndef _UDP_Component_h
#define _UDP_Component_h

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
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <pthread.h>
#include <deque>
#include <time.h>

using namespace std;

typedef double Timestamp;

#define MAX_MESSAGE_LEN 4096
#define MAX_PACKET_LEN 4096

#define NUM_THREADS     1

#define LEFT_LANE 5
#define RIGHT_LANE 0

#define MAX_NUM_OF_NODES 10

#define REQUEST_DENIED 0
#define REQUEST_GRANTED 1
#define REQUEST_ENTER_PLATOON 2	// A request to enter the car train
#define REQUEST_LEAVE_PLATOON 4	// A request to leave the platoon

#define LOCATION_PACKET 8 // Indicates this packet has updated location info
#define REQUEST_PACKET 16 // Indicates this packet is a request

// This is the struct that represents our packet
struct packet_to_send
{
	// Header
	unsigned int sequence_num;	// 4 bytes
	unsigned short source_address;	// 2 bytes
	unsigned short previous_hop;	// 2 bytes (source_address of last hop)
	unsigned short destination_address; // 2 bytes (All 1's means broadcast)
	Timestamp time_sent;	// 8 bytes
	
	// Payload
	unsigned short packet_type; // 2 bytes (Defines if update location packet or request packet)
	float x_position;	// 4 bytes
	int y_position;		// 4 bytes
	float x_speed;		// 4 bytes
	bool platoon_member;	// 1 byte (Indicates if this node is in a platoon)
	
	
} __attribute__((__packed__));

typedef struct packet_to_send tx_packet;

struct node_info
{
	int node_number;
	string node_hostname;
	string node_port_number;
	float node_x_coordinate;
	int node_y_coordinate;
	
	int number_of_links;
	
	string connected_hostnames[10];
	string connected_ports[10];
};

// Prototypes
//void send_packet(string , string , packet_to_send);

#endif
