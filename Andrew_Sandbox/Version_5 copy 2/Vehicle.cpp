//
//  Vehicle.cpp
//
//
//  Created by Andrew Marshall on 2/23/15.
//
//

#include <pthread.h>
#include <inttypes.h>
#include <math.h>
#include <time.h>
#include <iomanip>

#include "Resources.h"

#define DEBUG 1
//#undef DEBUG

using namespace std;

// The buffer used to store incoming packets
deque<tx_packet> Buffer;

// The boolean used to determine if it has been 10ms
bool can_transmit = false;
pthread_mutex_t tx_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Functions */

/*
 * Reads a Config.txt file and then populates the "nodes" data structure.
 *
 * @param: nodes: The data structure we are going to update.
 */
void read_node_info(node_info (&nodes)[MAX_NUM_OF_NODES + 1]) {
	
	ifstream myfile;
	
	string word;
	
	int node_num;
	
	int link_counter;
	string next_word;
	
	myfile.open("Config.txt");
	
	//	myfile.open(file_name_in);
	
	if (myfile.is_open())
	{
		while (myfile >> word) // Returns false if it can not read from file
		{
			if (word.compare("Node") == 0)
			{
				if (DEBUG) {
					//					cout << "Debug: NODE KEYWORD FOUND\n";
				}
				
				myfile >> node_num;
				nodes[node_num].node_number = node_num;
				
				myfile >> nodes[node_num].node_hostname;
				myfile >> nodes[node_num].node_port_number;
				myfile >> nodes[node_num].node_x_coordinate;
				myfile >> nodes[node_num].node_y_coordinate;
				
				if (0) {
				cout << "\nNode number: " << nodes[node_num].node_number << '\n';
				cout << "Host Name: " << nodes[node_num].node_hostname << '\n';
				cout << "Host Port: " << nodes[node_num].node_port_number << '\n';
				cout << "X Coordinate: " << nodes[node_num].node_x_coordinate << '\n';
				cout << "Y Coordinate: " << nodes[node_num].node_y_coordinate << '\n';
				}
				
				link_counter = 0;
				
				while (myfile.peek() != '\n')
				{
					myfile >> next_word;
					
					// Check to see if "links" is the next word
					if (next_word.compare("links") == 0) {
						
						cout << "Links attached to Node " << node_num << ": ";
					}
					
					else {
						
						link_counter++;
						
						if (DEBUG){
							//cout << "LINK FOUND!\n";
						}
						
						
						nodes[node_num].number_of_links = link_counter;
						
						if (DEBUG)
						{
							//							cout << "Number of Links found: " << nodes[node_num].number_of_links << '\n';
						}
						
						// We are past the links keyword, start reading in connected hostnames and port numbers
						
						nodes[node_num].connected_hostnames[link_counter - 1] = next_word;
						
						cout << nodes[node_num].connected_hostnames[link_counter - 1] << "(";
						
						myfile >> nodes[node_num].connected_ports[link_counter - 1];
						
						cout << nodes[node_num].connected_ports[link_counter - 1] << ") ";
					}
					
					
					// Check if we are at the end of the file
					if (myfile.peek() == EOF)
					{
						break;
					}
				}
				
				cout << '\n';
			}
			
		}
		
		myfile.close();
	}
	
	else
	{
		cout << "File does not exsist.\n";
	}
	
	
}

/*
 * Checks to see if a file exsists.
 */
bool file_exists(const char *filename)
{
	ifstream file_to_check;
	
	file_to_check.open(filename);
	
	if (file_to_check.fail())
	{
		file_to_check.close();
		return false;
	}
	
	file_to_check.close();
	return true;
	
}

/*
 * Determines if the Config.txt file exsists.
 * Then it determines what node number you are.
 */
int initial_config(string hostname_in, string portnum_in) {
	
	ofstream outStream;
	ifstream inStream;
	string line;
	int number_of_lines = 0;
	int node_number = 0;
	
	// Check to see if the file exists
	if (file_exists("Config.txt")) {
		
		if (DEBUG) {
			cout << "File Found!\n";
		}
		
		// Count how many lines there are to determine Node number
		inStream.open("Config.txt");
		
		while (getline(inStream, line))
		{
			number_of_lines++;
		}
		
		node_number = number_of_lines + 1;
		
		if (DEBUG) {
			cout << "Number of lines in file: " << number_of_lines << '\n';
			cout << "You are Node number: " << node_number << '\n';
		}
		inStream.close();
	}
	
	else
	{
		if (DEBUG) {
			cout << "File NOT Found!\n";
		}
		
		// Create new Config.txt file
		outStream.open("Config.txt");
		
		outStream << "Node 1 " << hostname_in << " " << portnum_in << " ";
		outStream << "0 0 " << "links";
		
		outStream.close();
		
		// Indicate that this is node 1
		node_number = 1;
	}
	
	
	return node_number;
}

/*
 * Rewrites the Config.txt File, using contents from "nodes" data structure.
 */
void rewrite_config_file(const node_info (&nodes)[MAX_NUM_OF_NODES + 1])
{
	ofstream outStream;
	
	outStream.open("Config.txt");
	
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		// If node number is -1 it is not a valid node.
		if (nodes[i].node_number != -1)
		{
			outStream << "Node " << nodes[i].node_number << " ";
			outStream << nodes[i].node_hostname << " ";
			outStream << nodes[i].node_port_number << " ";
			outStream << nodes[i].node_x_coordinate << " ";
			outStream << nodes[i].node_y_coordinate << " ";
			outStream << "links";
			
			for (int y = 0; y < nodes[i].number_of_links; y++)
			{
				outStream << " " << nodes[i].connected_hostnames[y] << " ";
				outStream << nodes[i].connected_ports[y];
			}
			
			// Add the new line
			outStream << '\n';
		}
	}
	
	outStream.close();
	
	
}

/*
 * Initializes our nodes data structure.
 */
void init_nodes(node_info (&nodes)[MAX_NUM_OF_NODES + 1])
{
	
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		nodes[i].node_number = -1;
		nodes[i].node_hostname = "N/A";
		nodes[i].node_port_number = "N/A";
		nodes[i].node_x_coordinate = -1000;
		nodes[i].node_y_coordinate = -1000;
		nodes[i].number_of_links = 0;
		
		for (int j = 0; j < MAX_NUM_OF_NODES; j++)
		{
			nodes[i].connected_hostnames[j] = "N/A";
			nodes[i].connected_ports[j] = "N/A";
			
		}
		
	}
	
}

/*
 * Used for debugging.
 * Displays the contents of the nodes data structure.
 */
void display_all_node_data(node_info (&nodes)[MAX_NUM_OF_NODES + 1])
{
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		cout << "nodes[" << i << "].node_number: "<< nodes[i].node_number << '\n';
		cout << "nodes[" << i << "].node_hostname: "<< nodes[i].node_hostname << '\n';
		cout << "nodes[" << i << "].node_port_number: "<< nodes[i].node_port_number << '\n';
		cout << "nodes[" << i << "].node_x_coordinate: "<< nodes[i].node_x_coordinate << '\n';
		cout << "nodes[" << i << "].node_y_coordinate: "<< nodes[i].node_y_coordinate << '\n';
		cout << "nodes[" << i << "].number_of_links: "<< nodes[i].number_of_links << '\n';
		
		for (int j = 0; j < MAX_NUM_OF_NODES; j++)
		{
			cout << "nodes[" << i << "].connected_hostnames[" << j << "]: "<< nodes[i].connected_hostnames[j] << '\n';
			cout << "nodes[" << i << "].connected_ports[" << j << "]: "<< nodes[i].connected_ports[j] << '\n';
		}
	}
}

/*
 * Used for debugging.
 * Displays the contents of the connected_nodes[].
 */
void display_all_connected_nodes(int connected_nodes[MAX_NUM_OF_NODES + 1])
{
	cout << "Connected Nodes: [ ";
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		cout << connected_nodes[i] << " ";
	}
	
	cout << "]\n";
}


// Used to determine if we are using IPv4 or IPv6
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	
	else
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

/*
 * Creates a thread that counts down "time_ms" time and then sets
 * the global boolean can_transmit to true
 */
void *timer(void *time_ms)
{
	while (1)
	{
		// Sleep for time_ms
		usleep(*static_cast<int*>(time_ms));
		
		// Lock the mutex
		pthread_mutex_lock(&tx_mutex);
		// Set global boolean to true
		can_transmit = true;
		// Unlock the mutex
		pthread_mutex_unlock(&tx_mutex);
	}
}

/*
 *	This starts a thread that listens for incoming packets and 
 *	puts them in a Queue.
 */
void *start_receiving(void *port_in)
{
	// Variables
	int sockfd;
	struct addrinfo hints, *servinfo, *p;
	int status;
	int numbytes;
	string my_port = *static_cast<string*>(port_in);
//	string my_port = (string)port_in;

	
	//struct sockaddr_storage their_addr;
	struct sockaddr_in their_addr;
	socklen_t addr_len;
	
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;	// set to AF_INIT to force IPv4
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_flags = AI_PASSIVE;	// use my IP
	
	
#ifdef DEBUG
	printf("DEBUG: Port number: %s\n", my_port.c_str());
	printf("Starting Server... to stop, press 'Ctrl + c'\n");
#endif
	
	// 1. getaddrinfo
	if ((status = getaddrinfo(NULL, my_port.c_str(),
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
	
#ifdef DEBUG
		printf("Binding complete, waiting to recvfrom...\n");
#endif
	
	addr_len = sizeof their_addr;
	
	while(1)
	{

		tx_packet packet_in;
		
		// 4. recvfrom
		// MAX_PACKET_LEN -1: To make room for '\0'
		if ((numbytes = recvfrom(sockfd, (char *) &packet_in, MAX_PACKET_LEN - 1, 0,
								 (struct sockaddr *)&their_addr, &addr_len)) == -1)
		{
			perror("recvfrom");
			exit(1);
		}
		
		// Put the packet in the Queue
		Buffer.push_back(packet_in);
		
#ifdef DEBUG
		printf("Packet Received! It contained: %d bytes.\n", numbytes);
		cout << "Pushing packet to end of buffer.\n";
#endif
		
		
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
		
//		close(sockfd);
	}
	
	close(sockfd);
	pthread_exit(NULL);
}


/*
 * Assumes at 100 meters we have a 0% chance of sending.
 *
 * @return A boolean indicating if the packet was sent (really received...)
 */
bool will_packet_send(float distance_apart) {

	bool send = false;
	
	float temp = distance_apart * distance_apart;
	
	float prob = ((10000 - temp) / 10000) * 10000;
	
//	srand(time(0));
	
	float chance = (rand() % 10001);
	
	if (chance < prob) {
		send = true;
	}
	
	if (DEBUG) {
		cout << "There is a " << prob << "/10,000 chance the packet will send.\n";
		cout << "RNG picked: " << chance << '\n';
		cout << "Will packet send: " << send << '\n';
	}
	
	return send;
}

/*
 * Initalizes the cache table.
 */
void init_cache_table(cache_table &cache_in)
{
	for (int i = 0; i < MAX_NUM_OF_NODES; i++)
	{
		cache_in.source_address[i] = 0;
		cache_in.highest_sequence_num[i] = 0;
		cache_in.number_of_broadcasts[i] = 0;
	}
}

/*
 * Used for debugging. Displays the cache table.
 */
void display_cache_table(cache_table const &cache_in)
{
	cout << "----- Cache Table -----\n";
	cout << "Index\tSource Address\tHighest Seq. Num.\tNum of Broadcasts\n";
	
	for (int i = 0; i < MAX_NUM_OF_NODES; i++)
	{
		cout << "[" << i << "]:\t\t" << cache_in.source_address[i];
		cout << "\t\t" << cache_in.highest_sequence_num[i];
		cout << "\t\t\t" << cache_in.number_of_broadcasts[i] << '\n';
	}
	
	cout << "----- End Cache Table -----\n";
}


int main(int argc, const char * argv[]) {
	
	// RNG - Seeded on System Time
	srand(time(0));
	
	/*	This holds all the Node info.
	*	NOTE: For ease of use the index will match the Node Number
	*	So nodes [0] will remain empty.
	*/
	node_info nodes[MAX_NUM_OF_NODES + 1];
	
	// This keeps track of which node numbers we are connected to
	// The index is the node number and '1' means we are connected.
	// Ex. {0,1,0,0,0,0,0,0,0,0,0,0} means we are connected to node 1
	
//	int connected_nodes[] = {0,0,0,0,0,0,0,0,0,0,0,0};

	// Create and initialicze Connected Nodes
	int connected_nodes[MAX_NUM_OF_NODES + 1];
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		connected_nodes[i] = 0;
	}
	
	// Create and initalize our Cache Table
	cache_table my_cache;
	init_cache_table(my_cache);
	
#ifdef DEBUG
	
	display_cache_table(my_cache);
	
#endif
	
	// This node's number
	int my_node_num;
	
	//----- Packet Header Variables -----
	// This nodes "unique" Address (4 bytes)
	unsigned int my_address;
	my_address = (rand() % 4000000001) + 1;
	
#ifdef DEBUG
		cout << "my_address: " << my_address << '\n';
#endif
	
	// Sequence Counter
	int sequence_counter = 0;
	
	// Platoon Indicator
	bool platoon_member = false;
	
	//----- Packet Header Variables End -----
	
	// The node's X and Y coordinate (temp because it is always being written)
	float x_temp;
	int y_temp;
	
	// The node's velocity along X Axis
	float speed_meter_per_sec = 30;
	
	// Initilaze your node list.
	init_nodes(nodes);
	
	if (argc != 3)
	{
		cout << "Error: Missing Params (Hostname, PortNumber)\n";
		exit(1);
	}
	
	// Get the host name and port number from Commnad Line
	string hostname = argv[1];
	string port_num = argv[2];
	
	// Initial Node set up, finds what node number you are
	my_node_num = initial_config(hostname, port_num);
	
	if (DEBUG) {
		cout << "Node Number: " << my_node_num << '\n';
	}
	
	// You are the first node to join and are thereby the Truck
	if (my_node_num == 1)
	{
		if (DEBUG) {
			cout << "I am a Truck!\n";
		}
		
		read_node_info(nodes);
		
		// Select semi-random starting point in range of [500, 800]
		x_temp = (rand() % 301) + 500;
		
#ifdef DEBUG
		// REMOVE - Using to test removal of links
//		x_temp = 500;
		cout << "Truck starting X-Coord: " << x_temp << '\n';
#endif
		
		// Always start in right lane
		y_temp = 0;
		
		// Select some semi-random speed in range of [20, 40]
		speed_meter_per_sec = (rand() % 21) + 20;
		
		if (DEBUG){
			cout << "Truck speed: " << speed_meter_per_sec << '\n';
		}
		
		// Not possbile for there to be any links because this is the first truck
		
		// Update the nodes hostname, port number, and node num
		nodes[my_node_num].node_number = my_node_num;
		nodes[my_node_num].node_hostname = hostname;
		nodes[my_node_num].node_port_number = port_num;
		nodes[my_node_num].node_x_coordinate = x_temp;
		nodes[my_node_num].node_y_coordinate = y_temp;
		
		// Rewrite file with new info
		rewrite_config_file(nodes);

	}
	
	else
	{
		// Read in node info to determine starting X and Y coord.
		read_node_info(nodes);
		
		// Logic to determine starting X and Y for this car
		bool safe_to_enter = true;
		
		do
		{
			// Assume it is safe each time we start the loop
			safe_to_enter = true;
			
			// Pick a random starting point in range of [0, 500]
			x_temp = (rand() % 501);
			
			// Check to make sure we are not within 15 meters of any other node
			for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
			{
				if ((x_temp + 15 > nodes[i].node_x_coordinate && x_temp < nodes[i].node_x_coordinate)
					|| (x_temp - 15 < nodes[i].node_x_coordinate && x_temp > nodes[i].node_x_coordinate))
				{
					if (DEBUG) {
						cout << "UN-SAFE SPOT SELECTED: " << x_temp << '\n';
						cout << "Too close to Node: " << i << " (" << nodes[i].node_x_coordinate << ")" << '\n';
					}
					safe_to_enter = false; // Mark it as Not Safe
					break;
				}
			}

		} while(!safe_to_enter); // Continue to look for new starting places until a safe spot is found
		
#ifdef DEBUG
		// REMOVE - Using to test removal of links
//		x_temp = 475;
		cout << "Car x_temp: " << x_temp << '\n';
		
#endif
		
		// Randomly pick if we are in the right or left lane
		if ((rand() % 101) < 50)
		{
			y_temp = RIGHT_LANE;
		}
		
		else {
			y_temp = LEFT_LANE;
		}
		
#ifdef DEBUG
			cout << "Car y_temp: " << y_temp << '\n';
#endif
		
		// Select some semi-random speed in range of [20, 40]
		speed_meter_per_sec = (rand() % 21) + 20;
		
		// Logic that determines what nodes are my initial links
		// We are in range if within 100 meters of another node
		
		int y = 0; // Use to increment the connected_hostname/port_num array
		
		for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
		{
			if ((x_temp + 100 > nodes[i].node_x_coordinate && x_temp < nodes[i].node_x_coordinate)
				|| (x_temp - 100 < nodes[i].node_x_coordinate && x_temp > nodes[i].node_x_coordinate))
			{
				if (DEBUG) {
					cout << "In Range of Node: " << i << '\n';
				}
				
				// Keep track of what nodes we are connected to.
				connected_nodes[i] = 1;
				
				nodes[my_node_num].number_of_links++; // Don't forget to set this!
				nodes[my_node_num].connected_hostnames[y] = nodes[i].node_hostname;
				nodes[my_node_num].connected_ports[y++] = nodes[i].node_port_number;
				
#ifdef DEBUG
				display_all_connected_nodes(connected_nodes);
#endif
				
			}
		}
		
		// Update the nodes data structure
		nodes[my_node_num].node_number = my_node_num;
		nodes[my_node_num].node_hostname = hostname;
		nodes[my_node_num].node_port_number = port_num;
		nodes[my_node_num].node_x_coordinate = x_temp;
		nodes[my_node_num].node_y_coordinate = y_temp;
		
		// Rewrite file with new info
		rewrite_config_file(nodes);
		
	}
	
//	display_all_node_data(nodes);
	
	
	// ----- Prepare MAIN LOOP -----
	
	pthread_t server_thread, timer_thread;
	
	
	// Create thread to listen for incoming packets
	int rc = pthread_create(&server_thread, NULL, start_receiving, &nodes[my_node_num].node_port_number);
	// check for creation errors
	if (rc)
	{
		cout << "Error: unable to create thread," << rc << endl;
		exit(-1);
	}
	
	// Create thread to listen for incoming packets
//	int micro_s = 10000; // Set the timer to 10 milliseconds
	int micro_s = 1500000; // Set the timer to 1.5 seconds
	rc = pthread_create(&timer_thread, NULL, timer, &micro_s);
	// check for creation errors
	if (rc)
	{
		cout << "Error: unable to create thread," << rc << endl;
		exit(-1);
	}
	
	// ----- BEGIN MAIN LOOP -----
	
	while (1)
	{
		
		// DEBUG ONLY
		// For debug purposes, I am putting this here so our output ins't so crazy!
		if (can_transmit) {
//		if (nodes[my_node_num].node_number == 1)
//		{
//			nodes[my_node_num].node_x_coordinate = nodes[my_node_num].node_x_coordinate + 10;
//			cout << "New Truck X Pos: " << nodes[my_node_num].node_x_coordinate << '\n';
//			rewrite_config_file(nodes);
//		}
		
		
		
		// Refresh our nodes data structure
		read_node_info(nodes);
		
		// Use the nodes data structure (from Config.txt) to determine what nodes I can connect to (<100m)
		for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
		{
			// Use the Config file to determine what nodes I can connect to (<100m)
			if ((nodes[my_node_num].node_x_coordinate + 100 > nodes[i].node_x_coordinate
				&& nodes[my_node_num].node_x_coordinate < nodes[i].node_x_coordinate)
				|| (nodes[my_node_num].node_x_coordinate - 100 < nodes[i].node_x_coordinate
				&& nodes[my_node_num].node_x_coordinate > nodes[i].node_x_coordinate))
			{
#ifdef DEBUG
					cout << "In Range of Node: " << i << '\n';
#endif
				
				// If this is a newly found node, need to keep track of it
				if (connected_nodes[i] == 0)
				{
					// Keep track of what nodes we are connected to.
					connected_nodes[i] = 1;
#ifdef DEBUG
					cout << "We have come into range of a new node.\n";
					display_all_connected_nodes(connected_nodes);
					display_all_node_data(nodes);
#endif
					
					
					// Update our "links" part of the nodes data structure
					nodes[my_node_num].number_of_links++; // Don't forget to set this!
					// Use number of links as our index for the hostname and portnumber arrays
					nodes[my_node_num].connected_hostnames[nodes[my_node_num].number_of_links - 1] = nodes[i].node_hostname;
					nodes[my_node_num].connected_ports[nodes[my_node_num].number_of_links - 1] = nodes[i].node_port_number;
					
				}
				
				else {
#ifdef DEBUG
					cout << "We are already connected to node " << i << ". Not adding to connected nodes.\n";
#endif
				}
			
				
#ifdef DEBUG
				display_all_connected_nodes(connected_nodes);
#endif
				
			}
			
			// Else we are not in range of that node. - Checks if we need to disconnect.
			else {
#ifdef DEBUG
				cout << "We are not in range of Node: " << i << '\n';
#endif
				// If we were connected previously, we need to disconnect
				if (connected_nodes[i] == 1)
				{
#ifdef DEBUG
					cout << "We are disconnecting from Node: " << i << '\n';
#endif
					connected_nodes[i] = 0;
					// nodes[my_node_num].number_of_links will tell us how many time we need to loop through the connected hostnames/ports
					// ports will be unique, that is how we will determine which index needs to be removed
					// Then we need to shuffle all the entires so they are left justified in the array... Linked List anyone?
					
					cout << "Number of links: " << nodes[my_node_num].number_of_links << '\n';
					display_all_node_data(nodes);
					
					for (int j = 0; j < nodes[my_node_num].number_of_links; j++)
					{
		
						if (nodes[my_node_num].connected_ports[j].compare(nodes[i].node_port_number) == 0)
						{
#ifdef DEBUG
							cout << "We need to remove index " << j << " from the connected_hostnames/ports.\n";
#endif
//							nodes[my_node_num].connected_hostnames[j] = "N/A";
//							nodes[my_node_num].connected_ports[j] = "N/A";
							
							// Shuffle the rest of the elements over (Overwritting j)
							while (j < nodes[my_node_num].number_of_links)
							{
								// Max number of links is 10 (can not have a link to yourself)
								// Therefore connected_hostnames/ports[MAX_NUM_OF_NODES] will always be N/A
								// So we do not need to worry about j+1 pointing out of bounds
								
								nodes[my_node_num].connected_hostnames[j] = nodes[my_node_num].connected_hostnames[j+1];
								nodes[my_node_num].connected_ports[j] = nodes[my_node_num].connected_ports[j+1];
								j++;
								
							}
							// Decrement our link counter
							nodes[my_node_num].number_of_links--;
							//display_all_node_data(nodes);
							// Break out of FOR loop!!! J is messed up now and will break the FOR loop if you don't.
							break;
						}
					}
					
				}
			}
		}
		
		
		// At this point our Node has updated all it's links.
		
		// if the Buffer isn't empty
		if (!Buffer.empty())
		{
			
			// Create next packet to service
			tx_packet packet_in;
			
#ifdef DEBUG
			cout << "Grabbing packet from front of queue.\n";
			cout << "Buffer Size: " << Buffer.size() << '\n';
#endif
			// Copy first item in queue
			packet_in = Buffer.front();
			
			// Pop the first packet in queue
			Buffer.pop_front();
			
			// ----- RBA STARTS HERE -----
			
			// TODO: Use RBA to forward packet to all my links
			// This is done no matter what kind of packet you are dealing with
			
			/*
			 Side note about cache:
			 - Cache Addresses are never removed.
			 - So even if some node goes out of range, we will remeber it in our cache
			 */
			
			// 1. Check Source Address of Packet
			int incoming_source_address = packet_in.source_address;
#ifdef DEBUG
			cout << "RBA: Incoming Source Address of packet: " << incoming_source_address << '\n';
//			cout << "Last Hop Port Number (int): " << packet_in.previous_hop_port << '\n';
//			
//			// Convert int to string
//			char str[15];
//			sprintf(str, "%d", packet_in.previous_hop_port);
//			
//			string test = string(str);
//			
//			cout << "Last Hop Port Number (string): " << test << '\n';
#endif
			// Make sure I am not the source
			if (incoming_source_address != my_address)
			{
				int cache_index = -1;
				// Need to find the index for that source number in the cache table
				for (int i = 0; i < MAX_NUM_OF_NODES; i++)
				{
					// If we have reached an empty entry
					if (my_cache.source_address[i] == 0)
					{
#ifdef DEBUG
						cout << "RBA: Adding new address (" << incoming_source_address;
						cout << ") to index: " << i << " of cache table.\n";
#endif
						// Add incoming source address to the cache
						my_cache.source_address[i] = incoming_source_address;
						
						// We know our index to be i
						cache_index = i;
						
						// Break out of the loop, our work here is done.
						break;
					}
					
					// Else if this is the droids, I mean index we are looking for
					else if (my_cache.source_address[i] == incoming_source_address)
					{
#ifdef DEBUG
						cout << "RBA: Found address (" << incoming_source_address;
						cout << ") at index: " << i << " in our cache table.\n";
#endif
						// We know our cache index
						cache_index = i;
						
						// Break out of the loop
						break;
					}
				}
				
				// 2. Check Cache Table to see if this is a new message
				//    Indicated by the seq. # being smaller in cache.
//				display_cache_table(my_cache);
//				cout << "my_cache.highest_sequence_num[cache_index]: " << my_cache.highest_sequence_num[cache_index] << '\n';
//				cout << "packet_in.sequence_num: " << packet_in.sequence_num << '\n';
				
				if (my_cache.highest_sequence_num[cache_index] < packet_in.sequence_num)
				{
					// This is a new packet
#ifdef DEBUG
					cout << "RBA: This is a new packet. (" << my_cache.highest_sequence_num[cache_index];
					cout << ") < (" << packet_in.sequence_num << ")\n";
#endif
					// 3. Update larget seq.# from this source in cache table
					my_cache.highest_sequence_num[cache_index] = packet_in.sequence_num;
					
					// 4. Set # times broadcasted to 1
					my_cache.number_of_broadcasts[cache_index] = 1;
					
					// 5. Forward this packet to all my links, except from where this came from
					// Using the packet_to_send.previous_hop_port to determine which one NOT to send to.
					// Used to convert int to string
					char str[15];
					// Convert int to string
					sprintf(str, "%d", packet_in.previous_hop_port);
					string prev_port_string = string(str);
					
					// Now that we have stored the prev port, we can update the last hop and prep the outgoing packet
					// Update the packet's last hop fields
					packet_in.previous_hop = my_address;
					packet_in.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
					
					// Now we can iterate through our links and send (if applicable)
					for (int y = 0; y < nodes[my_node_num].number_of_links; y++)
					{
#ifdef DEBUG
						cout << "RBA: Iterating through links: y = " << y << '\n';
#endif
						
						if (nodes[my_node_num].connected_ports[y].compare(prev_port_string) == 0)
						{
							// Do not send, this is the port where it came from
#ifdef DEBUG
							cout << "RBA: Do not send to port: " << prev_port_string << " this is where this packet came from.\n";
#endif
						}
						
						else
						{
							// Send packet to that port and hostname
#ifdef DEBUG
							cout << "RBA: Send to port: " << nodes[my_node_num].connected_ports[y];
							cout << " at address: " << nodes[my_node_num].connected_hostnames[y] << '\n';
#endif
							
							// Need to find which node number this is so we can determine the distance for the packet loss algorithim.
							
							int temp_node_num = 0; // 0 is an invalid node number
							
							for (int k=1; k < MAX_NUM_OF_NODES + 1; k++)
							{
								// If this is a connected node and the port number lines up with the port we want to send to
								if (connected_nodes[k] == 1
									&& (nodes[k].node_port_number).compare(nodes[my_node_num].connected_ports[y]) == 0)
								{
									// We know the node number is k
									temp_node_num = k;
									
#ifdef DEBUG
									cout << "RBA: Node Number: " << k << " corresponds to port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
								}
							}
							
							// Detetmine the distance from this node to it's neighbor
							float distance_apart = 0;
							distance_apart = nodes[my_node_num].node_x_coordinate - nodes[temp_node_num].node_x_coordinate;
							
#ifdef DEBUG
							cout << "RBA: Distance between nodes " << my_node_num;
							cout << " and " << temp_node_num << " is " << distance_apart << '\n';
#endif
							
							// Calculate "Packet Loss" probability (If > 100 meters, it can't send)
							if (will_packet_send(distance_apart))
							{
								//TODO: Keep track of a succsessful send
								
								// If get a green light, we send. Otherwise don't and move on to next neighbor
								
#ifdef DEBUG
								cout << "RBA: Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
								cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
								
								send_packet(nodes[my_node_num].connected_hostnames[y],
											nodes[my_node_num].connected_ports[y],
											packet_in);
							}
							
							else
							{
								//TODO: Keep track of the lost packet
							}
						}
					} // End FOR loop where we send packets.
					
				} // End IF checking if this is a new packet
				
				else if (my_cache.highest_sequence_num[cache_index] == packet_in.sequence_num)
				{
					//TODO: I think this behaviour can be accomplished in the above if statement
					// I have seen this packet and need to look at # times broadcasted to determine probability of resending.
					
					// Resend based on probability
					
					// Update # times broadcasted
					
				}
				
				else
				{
					// This is an old packet, and can be thrown away.
				}
				
			}
			else
			{
#ifdef DEBUG
				cout << "Received a packet from myself... Dropping packet.\n";
#endif
			}
			
			
			// TODO: Use this packet's info to update my state
			// Ex: Speed, lane position, send join platoon packet? Or if Truck deal with a recieved platoon request
			
			
			
			
			
			//		cout << "Speed: " << speed_meter_per_sec / 100.0 << '\n';
			
			//		x_temp = x_temp + (speed_meter_per_sec / 100);
			
			
			
			
#ifdef DEBUG
			cout << "Done servicing popped packet.\n";
			cout << "Buffer Size: " << Buffer.size() << '\n';
#endif
		} // End Popping and processing 1 packet from buffer
		
		// Write your new info (generated form both Config logic and packet buffer logic) to file
		// Config File - Updates Links
		// Packets - Updates Speed, lane position, and platoon forming
		rewrite_config_file(nodes);

		if (can_transmit)
		{
			// Create the status packet
			tx_packet packet_out;
			
			// Fill the status packet with Updated Data
			packet_out.sequence_num = sequence_counter++;
			packet_out.source_address = my_address;
			packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
			packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
			packet_out.destination_address = 0xFFFFFFFF; // All 1's means broadcast
			packet_out.time_sent = 0; // TODO: Need to find a way to track time in milli seconds or something
			packet_out.packet_type = LOCATION_PACKET;
			packet_out.x_position = nodes[my_node_num].node_x_coordinate;
			packet_out.y_position = nodes[my_node_num].node_y_coordinate;
			packet_out.x_speed = speed_meter_per_sec;
			packet_out.platoon_member = platoon_member;
			
#ifdef DEBUG
//			cout << "----- Packet to Send -----\n";
//			cout << "packet_out.sequence_num = " << packet_out.sequence_num << '\n';
//			cout << "packet_out.source_address = " << packet_out.source_address << '\n';
//			cout << "packet_out.previous_hop = " << packet_out.previous_hop << '\n';
//			cout << "packet_out.destination_address = " << packet_out.destination_address;
//			cout << " (" << hex << packet_out.destination_address << dec << ")\n"; // View in hex, then switch back to dec
//			cout << "packet_out.time_sent = " << packet_out.time_sent << '\n';
//			cout << "packet_out.packet_type = " << packet_out.packet_type << '\n';
//			cout << "packet_out.x_position = " << packet_out.x_position << '\n';
//			cout << "packet_out.y_position = " << packet_out.y_position << '\n';
//			cout << "packet_out.x_speed = " << packet_out.x_speed << '\n';
//			cout << "packet_out.platoon_member = " << packet_out.platoon_member << '\n';
#endif
			
			int y = 0; // Used to keep track of the Connected hostname / port numbers
			
			// Send a braodcast update packet to all attched nodes indicating it's new position and speed
			for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
			{
				// If we are connected to node i...
				if (connected_nodes[i] == 1)
				{
					// Detetmine the distance from this node to it's neighbor
					float distance_apart = 0;
					distance_apart = nodes[my_node_num].node_x_coordinate - nodes[i].node_x_coordinate;
					
#ifdef DEBUG
						cout << "Distance between nodes " << my_node_num;
						cout << " and " << i << " is " << distance_apart << '\n';
#endif
					
					// Calculate "Packet Loss" probability (If > 100 meters, it can't send)
					if (will_packet_send(distance_apart))
					{
						//TODO: Keep track of a succsessful send
						
						// If get a green light, we send. Otherwise don't and move on to next neighbor
						
						if (DEBUG) {
							cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
							cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
						}
						
						send_packet(nodes[my_node_num].connected_hostnames[y],
									nodes[my_node_num].connected_ports[y],
									packet_out);
					}
					
					else
					{
						//TODO: Keep track of the lost packet
					}
					
					// Increase our y counter to prep to send to the next connected node
					y++;
				}
			}
			
			// Lock the mutex
			pthread_mutex_lock(&tx_mutex);
			// Set global boolean to false
			can_transmit = false;
			// Unlock the mutex
			pthread_mutex_unlock(&tx_mutex);
			
		} // End can_transmit - IF

	} // End While
	} // End DEBUG IF Can_Transmit

	// kill threads
	pthread_exit(NULL);

	return 0;
	
}



































