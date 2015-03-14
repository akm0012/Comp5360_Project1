//
//  Resources.h
//
//
//  Created by Andrew Marshall on 2/25/15.
//  Refactored by Evan Hall on 3/12/15.
//
//
#ifndef RESOURCES_H
#define RESOURCES_H

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
#define INIT_LOCATION_PACKET 7 // Indicates this is a new node joining the sim
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

#define MAX_HOSTNAME_LENGTH 50


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <iomanip>
#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <pthread.h>
#include <deque>
#include <time.h>
using namespace std;


typedef double Timestamp;

// Used for timing and statistics 
typedef struct {

	time_t sec;
	long nsec;

} utime;

// This is the struct that represents our packet
struct packet_to_send {
	// Header
	unsigned int sequence_num;	// 4 bytes
	unsigned int source_address;	// 4 bytes
	unsigned int previous_hop;	// 4 bytes (address of last hop)
	int previous_hop_port;	// Port number of last hop
	unsigned int destination_address;	// 4 bytes (All 1's means broadcast)
	double time_sent;	// 8 bytes
	int node_number_source; // 4 bytes // Only used for status packets
	bool was_sent; // Used to keep track of if this was "sent in the simulator"

	// Payload
	unsigned short packet_type;	// 2 bytes (Defines if update location packet or request packet)
	float x_position;	// 4 bytes
	int y_position;		// 4 bytes
	float x_speed;		// 4 bytes
	bool platoon_member;	// 1 byte (Indicates if this node is in a platoon)
	int number_of_platoon_members;
	unsigned short message;	// 2 bytes
	
	int size_of_packet_except_hostname; // Size of everything in this packet, except the hostname
	char hostname[MAX_HOSTNAME_LENGTH];

} __attribute__(( __packed__ ));

typedef struct packet_to_send tx_packet;

struct node_info {

	int node_number;
	string node_hostname;
	string node_port_number;
	float node_x_coordinate;
	int node_y_coordinate;

	int number_of_links;

//	int connected_nodes[MAX_NUM_OF_NODES + 1];
	
	string connected_hostnames[MAX_NUM_OF_NODES];
	string connected_ports[MAX_NUM_OF_NODES];
};

struct cache_table {

	unsigned int source_address[MAX_NUM_OF_NODES];
	int highest_sequence_num[MAX_NUM_OF_NODES];
	int number_of_broadcasts[MAX_NUM_OF_NODES]; // # Times we have bradcasted highest seq.# packet
};

// Prototypes
extern void send_packet(string hostname_to_send, string port_to_send, packet_to_send packet_out);

static int get_number_of_digits(int number)
{
	int result;

	if (number < 0)
	{
		number *= -1;
		result++;
	}
	else if (number == 0)
	{
		return 1;
	}

	do
	{
		number /= 10;
		result++;

	} while (number != 0);
	
	return result;
}

static int get_number_of_digits(float number)
{
	int result;

	if (signbit(number))
	{
		number *= -1;
		result++;
	}
	else if (number == 0.0)
	{
		return 1;
	}

	double intpart, fractpart;
	fractpart = modf(number, &intpart);

	while (intpart != 0.0)
	{
		intpart /= 10;
		result++;
	}

	while (fractpart != 0.0)
	{
		fractpart *= 10;
		fractpart = modf(fractpart, &intpart);
		result++;
	}

	return result;
}

#endif