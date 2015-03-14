//
//  Resources.h
//  
//
//  Created by Andrew Marshall on 2/25/15.
//
//

#ifndef _Resources_h
#define _Resources_h

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

#define PASSING_SPEED_INCREASE 5 // 5 m/s
#define PASSING_BUFFER 20 

#define PLATOON_SPACE_BUFFER 15 // The space in between cars
#define TRUCK_LENGTH 10
#define CAR_LENGTH 5

#define DANGER_CLOSE 20

#define MAX_NUM_OF_NODES 11

// Packet Types
#define LOCATION_PACKET 8 // Indicates this packet has updated location info
#define REQUEST_PACKET 16 // Indicates this packet is a request
#define PLATOON_JOIN_INFO_PACKET 32 // Indicates this packet has platoon info for a joining car
#define PLATOON_WAIT_TO_JOIN_PACKET 33 // Indicates the car should wait to join
#define CAR_JOIN_STATUS 64 // Used to give the all clear

// Pre-defined Messages
#define ALL_CLEAR 20
#define REQUEST_DENIED 100
#define REQUEST_GRANTED 101
#define REQUEST_ENTER_PLATOON 50	// A request to enter the car train
#define REQUEST_LEAVE_PLATOON 51	// A request to leave the platoon

// Used for timing and statistics 
typedef struct {
	time_t sec;
	long nsec;
} utime;

// This is the struct that represents our packet
struct packet_to_send
{
	// Header
	unsigned int sequence_num;	// 4 bytes
	unsigned int source_address;	// 4 bytes
	unsigned int previous_hop;	// 4 bytes (address of last hop)
	int previous_hop_port; // Port number of last hop
	unsigned int destination_address; // 4 bytes (All 1's means broadcast)
	double time_sent;	// 8 bytes
	int node_number;
	
	// Payload
	unsigned short packet_type; // 2 bytes (Defines if update location packet or request packet)
	float x_position;	// 4 bytes
	int y_position;		// 4 bytes
	float x_speed;		// 4 bytes
	bool platoon_member;	// 1 byte (Indicates if this node is in a platoon)
	int number_of_platoon_members;
	unsigned short message; // 2 bytes
	
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
	
	string connected_hostnames[MAX_NUM_OF_NODES];
	string connected_ports[MAX_NUM_OF_NODES];
};

struct cache_table
{
	unsigned int source_address[MAX_NUM_OF_NODES];
	int highest_sequence_num[MAX_NUM_OF_NODES];
	int number_of_broadcasts[MAX_NUM_OF_NODES]; // # Times we have bradcasted highest seq.# packet
};

// Prototypes
extern void send_packet(string hostname_to_send, string port_to_send, packet_to_send packet_out);

#endif
