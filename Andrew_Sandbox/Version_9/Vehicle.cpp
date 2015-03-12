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
#include <signal.h>
#include <string>
#include <memory>

#include "Resources.h"

#define DEBUG 1
#define DEBUG_ROAD_RULES 1
#define DEBUG_PLATOON 1
#define PROD_OUTPUT 1
#undef DEBUG // Turn off DEBUG
#undef DEBUG_ROAD_RULES
#undef DEBUG_PLATOON
//#undef PROD_OUTPUT

using namespace std;

// The buffer used to store incoming packets
deque<tx_packet> Buffer;

// The boolean used to determine if it has been 10ms
bool can_transmit = false;
pthread_mutex_t tx_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Global Variables - Global so we can use them in out Ctrl-C Interrupt */
node_info nodes[MAX_NUM_OF_NODES + 1];
int connected_nodes[MAX_NUM_OF_NODES + 1];
cache_table my_cache;
const char *file_name;

// Used for reporting statistics
double response_time_sum;
double throughput_time_sum;
int total_number_of_responses;

int packets_sent_count;
int packets_lost_count;

/* Functions */

/*
 * Updates our response time sum.
 */
void update_response_time(double response_time_in)
{
	if (response_time_in >= 0)
	{
		response_time_sum = response_time_sum + response_time_in;
		total_number_of_responses++;
	}
	
}

/*
 * Updates our throughput time sum.
 */
void update_throughput_time(double throughput_time_in)
{
	if (throughput_time_in >= 0)
	{
		throughput_time_sum = throughput_time_sum + throughput_time_in;
	}
	
}

/*
 * Reads a Config.txt file and then populates the "nodes" data structure.
 *
 * @param: nodes: The data structure we are going to update.
 */
void read_node_info(node_info (&nodes)[MAX_NUM_OF_NODES + 1])
{
	
	ifstream myfile;
	
	string word;
	
	int node_num;
	
	int link_counter;
	string next_word;
	
	myfile.open(file_name);
	
	//	myfile.open(file_name_in);
	
	if (myfile.is_open())
	{
		while (myfile >> word) // Returns false if it can not read from file
		{
			if (word.compare("Node") == 0)
			{
#ifdef DEBUG
				//					cout << "Debug: NODE KEYWORD FOUND\n";
#endif
				
				myfile >> node_num;
				nodes[node_num].node_number = node_num;
				
				myfile >> nodes[node_num].node_hostname;
				myfile >> nodes[node_num].node_port_number;
				myfile >> nodes[node_num].node_x_coordinate;
				myfile >> nodes[node_num].node_y_coordinate;
				
				link_counter = 0;
				
				nodes[node_num].number_of_links = link_counter;
				
				// Reset our Connected Ports and Hostnames
				for (int i = 0; i < MAX_NUM_OF_NODES; i++)
				{
					nodes[node_num].connected_hostnames[i] = "N/A";
					nodes[node_num].connected_ports[i] = "N/A";
				}
				
				if (0) {
					cout << "\nNode number: " << nodes[node_num].node_number << '\n';
					cout << "Host Name: " << nodes[node_num].node_hostname << '\n';
					cout << "Host Port: " << nodes[node_num].node_port_number << '\n';
					cout << "X Coordinate: " << nodes[node_num].node_x_coordinate << '\n';
					cout << "Y Coordinate: " << nodes[node_num].node_y_coordinate << '\n';
				}
				
				//link_counter = 0;
				
				while (myfile.peek() != '\n')
				{
					myfile >> next_word;
					
					// Check to see if "links" is the next word
					if (next_word.compare("links") == 0) {
#ifdef DEBUG
						//cout << "Links attached to Node " << node_num << ": ";
#endif
					}
					
					else {
						
						link_counter++;
						
#ifdef DEBUG
						//cout << "LINK FOUND!\n";
#endif
						
						
						nodes[node_num].number_of_links = link_counter;
						
#ifdef DEBUG
						// cout << "Number of Links found: " << nodes[node_num].number_of_links << '\n';
#endif
						
						// We are past the links keyword, start reading in connected hostnames and port numbers
						
						nodes[node_num].connected_hostnames[link_counter - 1] = next_word;
#ifdef DEBUG
						//					cout << nodes[node_num].connected_hostnames[link_counter - 1] << "(";
#endif
						myfile >> nodes[node_num].connected_ports[link_counter - 1];
#ifdef DEBUG
						//					cout << nodes[node_num].connected_ports[link_counter - 1] << ") ";
#endif
					}
					
					
					// Check if we are at the end of the file
					if (myfile.peek() == EOF)
					{
						break;
					}
				}
#ifdef DEBUG
				//			cout << '\n';
#endif
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
 * Gets our port number as a string depending on what node number we are.
 */
string get_port_num(int node_num_in)
{
	switch (node_num_in) {
  case 1:
			return "10130";
  case 2:
			return "10131";
  case 3:
			return "10132";
  case 4:
			return "10133";
  case 5:
			return "10134";
  case 6:
			return "10135";
  case 7:
			return "10136";
  case 8:
			return "10137";
  case 9:
			return "10138";
  case 10:
			return "10139";
  case 11:
			return "10140";
  default:
			break;
	}
	
	return "null";
}


/*
 * Determines if the Config.txt file exsists.
 * Then it determines what node number you are.
 */
int initial_config(string hostname_in)
{
	
	ofstream outStream;
	ifstream inStream;
	string line;
	int number_of_lines = 0;
	int node_number = 0;
	
	// Check to see if the file exists
	if (file_exists(file_name)) {
		
#ifdef DEBUG
		cout << "File Found!\n";
#endif
		
		// Count how many lines there are to determine Node number
		inStream.open(file_name);
		
		while (getline(inStream, line))
		{
			number_of_lines++;
		}
		
		node_number = number_of_lines + 1;
		
#ifdef DEBUG
		cout << "Number of lines in file: " << number_of_lines << '\n';
		cout << "You are Node number: " << node_number << '\n';
#endif
		inStream.close();
	}
	
	else
	{
#ifdef DEBUG
		cout << "File NOT Found!\n";
#endif
		
		// Create new Config.txt file
		outStream.open(file_name);
		
		outStream << "Node 1 " << hostname_in << " " << get_port_num(node_number)
 << " ";
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
	
	outStream.open(file_name);
	
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
	cout << "----- All Node Data -----\n";
	for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
	{
		if (nodes[i].node_hostname.compare("N/A") != 0)
		{
			cout << "nodes[" << i << "].node_number: "<< nodes[i].node_number << '\n';
			cout << "nodes[" << i << "].node_hostname: "<< nodes[i].node_hostname << '\n';
			cout << "nodes[" << i << "].node_port_number: "<< nodes[i].node_port_number << '\n';
			cout << "nodes[" << i << "].node_x_coordinate: "<< nodes[i].node_x_coordinate << '\n';
			cout << "nodes[" << i << "].node_y_coordinate: "<< nodes[i].node_y_coordinate << '\n';
			cout << "nodes[" << i << "].number_of_links: "<< nodes[i].number_of_links << '\n';
		}
		
		
		for (int j = 0; j < MAX_NUM_OF_NODES; j++)
		{
			if (nodes[i].connected_hostnames[j].compare("N/A") != 0)
			{
				cout << "nodes[" << i << "].connected_hostnames[" << j << "]: "<< nodes[i].connected_hostnames[j] << '\n';
				cout << "nodes[" << i << "].connected_ports[" << j << "]: "<< nodes[i].connected_ports[j] << '\n';
			}
			
		}
	}
}

/*
 * Used for debugging.
 * Displays the contents of the connected_nodes[].
 */
void display_all_connected_nodes(int connected_nodes[MAX_NUM_OF_NODES + 1])
{
	cout << "----- Connected Nodes -----\n";
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
	
#ifdef DEBUG
	printf("Sent %d bytes to %s\n", numbytes_tx, hostname_to_send.c_str());
#endif
	
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
bool will_packet_send(float distance_apart)
{
	
	bool send = false;
	
	float temp = distance_apart * distance_apart;
	
	float prob = ((10000 - temp) / 10000) * 10000;
	
	//	srand(time(0));
	
	float chance = (rand() % 10001);
	
	if (chance < prob) {
		send = true;
	}
	
#ifdef DEBUG
	cout << "There is a " << prob << "/10,000 chance the packet will send.\n";
	cout << "RNG picked: " << chance << '\n';
	cout << "Will packet send: " << send << '\n';
#endif
	
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

/*
 * This is used to determine if we should forward a packet.
 *
 *@param num_of_broadcast The number of times we have already broadcast.
 */
bool will_rebraodcast(int num_of_broadcast)
{
	float percent = 50.0;
	bool send = false;
	
	for (int i = 1; i < num_of_broadcast; i++)
	{
		percent = percent / 2;
	}
	
#ifdef DEBUG
	cout << "RBA: num_of_braodcasts: " << num_of_broadcast << '\n';
	cout << "RBA: percent: " << percent << '\n';
#endif
	
	float prob = percent * 100;
	
	float chance = (rand() % 10001);
	
	if (chance < prob) {
		send = true;
	}
	
#ifdef DEBUG
	cout << "RBA: There is a " << prob << "/10,000 chance the packet will send.\n";
	cout << "RBA: RNG picked: " << chance << '\n';
	cout << "RBA: Will packet send: " << send << '\n';
#endif
	
	return send;
}

/*
 * Returns the distance traveled after some
 * time measured in micro seconds.
 */
float get_distance_traveled(float speed_meter_per_sec_in, float time_micro_s)
{
	return speed_meter_per_sec_in * (0.000001) * time_micro_s;
}

/*
 * Sets your new location given your old one.
 */
void set_new_location(float &x_coord_in, float speed_meter_per_sec_in, float time_micro_s)
{
	//	cout << "DEBUG: x_coord_in OLD: " << x_coord_in << '\n';
	//	cout << "DEBUG: distance travled: " << get_distance_traveled(speed_meter_per_sec_in, time_micro_s) << '\n';
	x_coord_in = x_coord_in + get_distance_traveled(speed_meter_per_sec_in, time_micro_s);
	//	cout << "DEBUG: x_coord_in NEW: " << x_coord_in << '\n';
}

/*
 * Finds the distance I should be from the lead truck
 */
float find_distance_from_truck(int num_in_train)
{
	// Truck counts as 1
	// 25 + ((num_in_train -2) * 20)
	
	// See My Notes for more details on how this works
	
	//cout << "DEBUG: num_in_train: " << num_in_train << '\n';
	
	//cout << "DEBUG: Returning: " << (PLATOON_SPACE_BUFFER + TRUCK_LENGTH) + ((num_in_train - 2) * (PLATOON_SPACE_BUFFER + CAR_LENGTH)) << '\n';
	
	return (PLATOON_SPACE_BUFFER + TRUCK_LENGTH) + ((num_in_train - 2) * (PLATOON_SPACE_BUFFER + CAR_LENGTH));
}

/*****************************************************************************\
 * This function is called whenever the associated signal (event) occurs       *
 * This function will handle the event                                         *
 \*****************************************************************************/
static void signalHandler(int signo)
{
	// signal(signo,SIG_IGN);
	signal(SIGINT,signalHandler); // needed on some systems
	
	cout<< "\n---------- Statistics ----------\n";
	
	// Calculate our averages
	double average_throughput = 0;
	double average_response_time = 0;
	float average_packet_loss_percent = 0;
	
	if (total_number_of_responses != 0)
	{
		average_throughput = throughput_time_sum / total_number_of_responses;
		average_response_time = response_time_sum / total_number_of_responses;
		average_packet_loss_percent = packets_lost_count / ((float)packets_sent_count + (float)packets_lost_count);
		average_packet_loss_percent = average_packet_loss_percent * 100;

		cout << "Average Latency (Response Time):\t" << average_response_time << "\n";
		
		cout << "Average Throughput Time:\t\t" << average_throughput << "\n";
		
		cout << "Average Packet Loss:\t\t\t" << average_packet_loss_percent << "%\n";

	}
	
	else
	{
		cout << "Average Latency (s):\t" << "No Packets were received.\n";
		
		cout << "Average Throughput Time (s):\t\t" << "No Packets were received.\n";
		
		cout << "Average Packet Loss:\t\t\t" << "No Packets were received.\n";

	}
	
	
	
	
	//display_all_node_data(nodes);
	
	//display_cache_table(my_cache);
	
	//display_all_connected_nodes(connected_nodes);
	
	//cout << "Buffer Size: " << Buffer.size() << '\n';
	
	exit(0);
	
	
	return;
}

/*
 * Lets us determine if the left or right lane are clear.
 */
bool check_if_lane_clear(int lane_in, const node_info (&nodes)[MAX_NUM_OF_NODES + 1], int my_node_num_in)
{
	bool clear = true;
	float x_temp = nodes[my_node_num_in].node_x_coordinate;
	
	// We are checking if the Right Lane is clear
	if (lane_in == RIGHT_LANE)
	{
		// Check to make sure we are not within PASSING_BUFFER meters of any other node in the right lane
		for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
		{
			if ((((x_temp + PASSING_BUFFER >= nodes[i].node_x_coordinate && x_temp <= nodes[i].node_x_coordinate)
				  || (x_temp - PASSING_BUFFER <= nodes[i].node_x_coordinate && x_temp >= nodes[i].node_x_coordinate)))
				&& (nodes[i].node_y_coordinate == RIGHT_LANE))
			{
#ifdef DEBUG
				cout << "Not Safe to enter Right Lane at: " << x_temp << '\n';
				cout << "Too close to Node: " << i << " (" << nodes[i].node_x_coordinate << ")" << '\n';
#endif
				clear = false; // Mark it as Not Safe
				break;
			}
		}
	}
	
	// We are checking if the Left Lane is clear
	else if (lane_in == LEFT_LANE)
	{
		// Check to make sure we are not within 15 meters of any other node in the left lane
		for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
		{
			if ((((x_temp + PASSING_BUFFER >= nodes[i].node_x_coordinate && x_temp <= nodes[i].node_x_coordinate)
				  || (x_temp - PASSING_BUFFER <= nodes[i].node_x_coordinate && x_temp >= nodes[i].node_x_coordinate)))
				&& (nodes[i].node_y_coordinate == LEFT_LANE))
			{
#ifdef DEBUG
				cout << "Not Safe to enter Left Lane at: " << x_temp << '\n';
				cout << "Too close to Node: " << i << " (" << nodes[i].node_x_coordinate << ")" << '\n';
#endif
				clear = false; // Mark it as Not Safe
				break;
			}
		}
	}
	
	else
	{
		cout << "ERROR: check_if_lane_clear: invalid param for lane: " << lane_in << '\n';
		exit(0);
	}
	
	return clear;
}


void rewrite_config_file(string lineToUpdate)
{
	// Evan Magic
	unique_ptr<char[]> lineArray(new char[lineToUpdate.length() + 1]);
	strncpy(lineArray.get(), lineToUpdate.c_str(), lineToUpdate.length() + 1);
	
	string writeThis;
	
	char *parsedWord;
	strtok(lineArray.get(), " ");
	parsedWord = strtok(NULL, " ");
	
	string convParsedWord(parsedWord);
	int lineNumber = stoi(convParsedWord);
	
	ifstream inStream(file_name);
	
	int currentLine = 1;
	string line;
	while (getline(inStream, line))
	{
		if (currentLine == lineNumber)
		{
			//lineToUpdate.append("\n");
			writeThis.append(lineToUpdate);
		}
		
		else
		{
			writeThis.append(line + '\n');
		}
		
		currentLine++;
		line = "";
	}
	inStream.close();
	
	ofstream outStream;
	
	outStream.open(file_name);
	
	outStream.write(writeThis.c_str(), writeThis.size());
	
	outStream.close();
}

/*
 Used for time tracking. 
 */
utime get_time_from_epoch()
{
	utime result;
	struct timespec spec;
	
	clock_gettime(CLOCK_REALTIME, &spec);
	
	result.sec = spec.tv_sec;
	result.nsec = round(spec.tv_nsec / 10e6);
	
	return result;
}

/*
 * Returns the current time as a double. 
 */
double get_time()
{
	// Get the time the packet was sent
	utime now = get_time_from_epoch();
	
	// Get the time as a string
	string time_stamp_out_string = "";
	time_stamp_out_string.append(to_string(now.sec));
	time_stamp_out_string = time_stamp_out_string.substr(6, time_stamp_out_string.length());
	time_stamp_out_string.append(".");
	time_stamp_out_string.append(to_string(now.nsec));
	
	// Return it as a double
	return stod(time_stamp_out_string);
}


int main(int argc, const char * argv[])
{
	
	// Used for Debugging - Catches Ctrl-C
	signal(SIGINT, signalHandler); // Prepare code to handle a Ctrl + c interrupt
	
	// Used for reporting statistics (Declared as Global Variables)
	response_time_sum = 0;
	throughput_time_sum = 0;
	total_number_of_responses = 0;
	
	packets_sent_count = 0;
	packets_lost_count = 0;
	
	// RNG - Seeded on System Time
	srand(time(0));
	
	/*	This holds all the Node info.
	 *	NOTE: For ease of use the index will match the Node Number
	 *	So nodes [0] will remain empty.
	 */
	// GLOBAL	node_info nodes[MAX_NUM_OF_NODES + 1];
	
	// This keeps track of which node numbers we are connected to
	// The index is the node number and '1' means we are connected.
	// Ex. {0,1,0,0,0,0,0,0,0,0,0,0} means we are connected to node 1
	// GLOBAL	int connected_nodes[MAX_NUM_OF_NODES + 1];
	
	// Create and initialicze Connected Nodes
	for (int i = 0; i < MAX_NUM_OF_NODES + 1; i++)
	{
		connected_nodes[i] = 0;
	}
	
	// Create and initalize our Cache Table
	// GLOBAL	cache_table my_cache;
	init_cache_table(my_cache);
	
	// This node's number
	int my_node_num;
	
	//----- Variables -----
	
	// Unique Address ID
	unsigned int my_address;
	
	// Sequence Counter
	int sequence_counter = 0;
	
	// Platoon Indicator
	bool platoon_member = false;
	
	// Used to indicate I should maintain some speed other than my original
	bool holding_pattern = false;
	
	// Used to indicate if the platoon is open (Truck only)
	bool platoon_open = true;
	
	// Used to keep track of the last car that asked to join the platoon (in case the packet is lost) (Truck only)
	unsigned int address_of_last_car_to_try_and_join_platoon = 0;
	
	// Keeps track of how many cars are in the platoon
	int platoon_member_count = 0; // If we are truck this will go to 1
	
	// Used for timing
	double time_in_temp = 0;
	
	//----- Packet Header Variables End -----
	
	// The node's X and Y coordinate (temp because it is always being written)
	float x_temp;
	int y_temp;
	
	// The node's velocity along X Axis
	float starting_speed_meter_per_sec = 0;
	float speed_meter_per_sec = 0;
	
	// Used for rewriting the text file
	string new_line;
	
	// Initilaze your node list.
	init_nodes(nodes);
	
	if (argc != 2)
	{
		cout << "Error: Missing Params (PortNumber)\n";
		exit(1);
	}
	
	// Get the host name and port number from Commnad Line
	
	char host_name[_SC_HOST_NAME_MAX];
	gethostname(host_name, _SC_HOST_NAME_MAX);
	
	// Get the hostname
	string hostname(host_name);
	hostname.shrink_to_fit();
	
	//string port_num = argv[1];
	file_name = argv[1];
	
	// Initial Node set up, finds what node number you are
	my_node_num = initial_config(hostname);
	
	// Get our port number depending on what node we are
	string port_num = get_port_num(my_node_num);
	
#ifdef DEBUG
	cout << "Node Number: " << my_node_num << '\n';
#endif
	
	// This nodes "unique" Address (4 bytes)
	my_address = (rand() % 4000000001) + my_node_num;
	
#ifdef DEBUG_ROAD_RULES
	cout << "my_address: " << my_address << '\n';
#endif
	
	
	// You are the first node to join and are thereby the Truck
	if (my_node_num == 1)
	{
#ifdef DEBUG
		cout << "I am a Truck!\n";
#endif
		
		// Indicate we have started a platoon.
		platoon_member_count = 1;
		platoon_member = true;
		
		// Don't think we need this... Investigate later
		//read_node_info(nodes);
		
		// Select semi-random starting point in range of [500, 800]
		x_temp = (rand() % 301) + 500;
		
#ifdef DEBUG
		cout << "Truck starting X-Coord: " << x_temp << '\n';
#endif
		
		// Always start in right lane
		y_temp = RIGHT_LANE;
		
		// Select some semi-random speed in range of [25, 30]
		speed_meter_per_sec = (rand() % 6) + 25;
		starting_speed_meter_per_sec = speed_meter_per_sec;
		
		// -- DEBUG CODE --
		// Can be used to set up custom scenarios
//		cout << "DEBUG: Truck 1\n";
//		y_temp = RIGHT_LANE;
//		speed_meter_per_sec = 22;
//		starting_speed_meter_per_sec = speed_meter_per_sec;
//		x_temp = 1054;
		
#ifdef DEBUG
		cout << "Truck speed: " << speed_meter_per_sec << '\n';
#endif
		
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
	
	// You are a car
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
				if ((x_temp + 15 >= nodes[i].node_x_coordinate && x_temp <= nodes[i].node_x_coordinate)
					|| (x_temp - 15 <= nodes[i].node_x_coordinate && x_temp >= nodes[i].node_x_coordinate))
				{
#ifdef DEBUG
					cout << "UN-SAFE SPOT SELECTED: " << x_temp << '\n';
					cout << "Too close to Node: " << i << " (" << nodes[i].node_x_coordinate << ")" << '\n';
#endif
					safe_to_enter = false; // Mark it as Not Safe
					break;
				}
			}
			
		} while(!safe_to_enter); // Continue to look for new starting places until a safe spot is found
		
#ifdef DEBUG
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
		
		// Select some semi-random speed in range of [30, 45]
		speed_meter_per_sec = (rand() % 16) + 30;
		starting_speed_meter_per_sec = speed_meter_per_sec;
	
		// -- DEBUG CODE --
		// Can be used to set up custom scenarios
		/*
		// Car 2 (Truck counts as 1)
		if (my_node_num == 2)
		{
			cout << "DEBUG: Car 2\n";
			y_temp = RIGHT_LANE;
			speed_meter_per_sec = 30;
			starting_speed_meter_per_sec = speed_meter_per_sec;
			x_temp = 758;
		}
		
		// Car 3
		if (my_node_num == 3)
		{
			cout << "DEBUG: Car 3\n";
			y_temp = LEFT_LANE;
			speed_meter_per_sec = 38;
			starting_speed_meter_per_sec = speed_meter_per_sec;
			x_temp = 486;
		}
		
		// Car 4
		if (my_node_num == 4)
		{
			cout << "DEBUG: Car 4\n";
			y_temp = RIGHT_LANE;
			speed_meter_per_sec = 28;
			starting_speed_meter_per_sec = speed_meter_per_sec;
			x_temp = 470;
		}
		
		// Car 5
		if (my_node_num == 5)
		{
			cout << "DEBUG: Car 5\n";
			y_temp = RIGHT_LANE;
			speed_meter_per_sec = 28;
			starting_speed_meter_per_sec = speed_meter_per_sec;
			x_temp = 470;
		}
		
		// Car 6
		if (my_node_num == 6)
		{
			cout << "DEBUG: Car 6\n";
			y_temp = RIGHT_LANE;
			speed_meter_per_sec = 28;
			starting_speed_meter_per_sec = speed_meter_per_sec;
			x_temp = 470;
		}
		*/
		
		// Logic that determines what nodes are my initial links
		// We are in range if within 100 meters of another node
		
		int y = 0; // Use to increment the connected_hostname/port_num array
		
		for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
		{
			if (((x_temp + 100 >= nodes[i].node_x_coordinate && x_temp <= nodes[i].node_x_coordinate)
				 || (x_temp - 100 <= nodes[i].node_x_coordinate && x_temp >= nodes[i].node_x_coordinate))
				&& (i != my_node_num)) // Makes sure we can't create a link to ourselves.
			{
#ifdef DEBUG
				cout << "In Range of Node: " << i << '\n';
#endif
				
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
	int micro_s = 10000; // Set the timer to 10 milliseconds
	//int micro_s = 100000; // Set the timer to 100 milliseconds
	//int micro_s = 1000000; // Set the timer to 1 second
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
		//		if (can_transmit) {
		
		// MOVED LINK ADDITION / DELETION TO END (Inside timer)
		
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
			
			// Record the Response Time
			update_response_time(get_time() - packet_in.time_sent);
			//cout << setprecision(14) << "Response Time: " << get_time() - packet_in.time_sent << '\n';
			
			// ----- RBA STARTS HERE -----
		
			/*
			 Side note about cache:
			 - Cache Addresses are never removed.
			 - So even if some node goes out of range, we will remeber it in our cache
			 */
			
			// 1. Check Source Address of Packet
			int incoming_source_address = packet_in.source_address;
#ifdef DEBUG
			cout << "RBA: Incoming Source Address of packet: " << incoming_source_address << '\n';
			cout << "RBA: My Address: " << my_address << '\n';
#endif
			// Make sure I am not the source
			if (incoming_source_address != my_address)
			{
				bool shall_I_forward = true;
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
				
				if (my_cache.highest_sequence_num[cache_index] <= packet_in.sequence_num)
				{
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
						
						shall_I_forward = true;
					}
					
					else if (my_cache.highest_sequence_num[cache_index] == packet_in.sequence_num)
					{
						// This needs to be rebroadcasted depending on the rebroadcast alg.
#ifdef DEBUG
						cout << "RBA: This is the SAME packet. (" << my_cache.highest_sequence_num[cache_index];
						cout << ") == (" << packet_in.sequence_num << ")\n";
#endif
						
						// Use the number of rebroadcasts to determine if we should re-broadcast
						shall_I_forward = will_rebraodcast(my_cache.number_of_broadcasts[cache_index]);
						
						if (shall_I_forward)
						{
							// Increment our broadcast counter
							my_cache.number_of_broadcasts[cache_index]++;
						}
						
					}
					
					if (shall_I_forward)
					{
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
								distance_apart = x_temp - nodes[temp_node_num].node_x_coordinate;
								
#ifdef DEBUG
								cout << "RBA: Distance between nodes " << my_node_num;
								cout << " and " << temp_node_num << " is " << distance_apart << '\n';
#endif
								
								// Calculate "Packet Loss" probability (If > 100 meters, it can't send)
								if (will_packet_send(distance_apart))
								{
									packets_sent_count++;
									
									// If get a green light, we send. Otherwise don't and move on to next neighbor
									
#ifdef DEBUG
									cout << "RBA: Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
									cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
									
									// Update the new sent time time
									time_in_temp = packet_in.time_sent;
									
									packet_in.time_sent = get_time();
									
									send_packet(nodes[my_node_num].connected_hostnames[y],
												nodes[my_node_num].connected_ports[y],
												packet_in);
								}
								
								else
								{
									packets_lost_count++;
								}
							}
						} // End FOR loop where we send packets.
					} // End If for Shall_I_Forward
					
				} // End IF checking if this is a new packet
				
				else
				{
					// This is an old packet, and can be thrown away.
#ifdef DEBUG
					cout << "Old packet, throwing it out...\n";
#endif
				}
				
				// Record the throughput time (aka Turn Around Time)
				update_throughput_time(get_time() - time_in_temp);
			}
			else
			{
#ifdef DEBUG
				cout << "Received a packet from myself... Dropping packet.\n";
#endif
				// Record the throughput time (aka Turn Around Time)
				update_throughput_time(get_time() - packet_in.time_sent);
			}
			
			
			// TODO: Use this packet's info to update my state
			// Ex: Speed, lane position, send join platoon packet? Or if Truck deal with a recieved platoon request
			
			// Make sure we arn't servicing a packet from ourselves
			if (incoming_source_address != my_address)
			{
				// Checks to see if it's a Status packet (not a platoon related packet)
				if (packet_in.packet_type == LOCATION_PACKET)
				{
					// IF I get a packet indicating a car is less than 20 meters ahead (same lane) AND is going slower than me
					// AND is NOT in a Platoon
					if (packet_in.x_position <= x_temp + DANGER_CLOSE // Car ahead is less than DANGER_CLOSE
						&& packet_in.x_position > x_temp // Makes sure this car is ahead, not behind
						&& packet_in.y_position == y_temp // In same lane
						&& packet_in.x_speed <= speed_meter_per_sec // <= so this will keep being executed until LEFT LANE clear
						&& packet_in.platoon_member == false // Not a car train
						&& platoon_member == false)
					{
#ifdef DEBUG_ROAD_RULES
						cout << "-- ROAD RULES: Danger Close Car ahead! --\n";
						cout << "Buffer Size: " << Buffer.size() << '\n';
						cout << "My X: " << x_temp << '\n';
						cout << "Other Vechicle X: " << packet_in.x_position << '\n';
#endif
						// If I am in the RIGHT LANE
						if (y_temp == RIGHT_LANE)
						{
							// Check if the LEFT LANE is clear
							if (check_if_lane_clear(LEFT_LANE, nodes, my_node_num))
							{
								// Leave a holding pattern
								holding_pattern = false;
#ifdef DEBUG_ROAD_RULES
								cout << "-- ROAD RULES: Left Lane is clear. Moving into Left Lane to pass.\n";
								cout << "Buffer Size: " << Buffer.size() << '\n';
#endif
								// Move into the left lane
								y_temp = LEFT_LANE;
							}
							
							// Left Lane is not clear, match ahead car's speed until it is clear
							else
							{
								// Enter a holding pattern
								holding_pattern = true;
								
								// Match car's speed
								speed_meter_per_sec = packet_in.x_speed;
								
								
								
								// Might have to teleport to exactly 20 m away so this case keeps calling
								// POTENTIAL PROBLEM: This should fix the issue when a car matches the speed of the car ahead,
								// but does so where he stays more than danger close distance away.
								x_temp = packet_in.x_position - 19;
#ifdef DEBUG_ROAD_RULES
								cout << "-- ROAD RULES: Left Lane is NOT Clear, matching speed.\n";
								cout << "New Speed: " << speed_meter_per_sec << '\n';
#endif
							}
						}
						
						// Else if I am in the LEFT LANE and Danger Close
						else if (y_temp == LEFT_LANE)
						{
							// Enter a holding pattern
							holding_pattern = true;
							
							// Match speed of car ahead of me
							speed_meter_per_sec = packet_in.x_speed;
							
							//TODO: May need to teleport
#ifdef DEBUG_ROAD_RULES
							cout << "-- ROAD RULES: DANGER CLOSE In LEFT Lane. --\n";
							cout << "Matching Speed.\n";
							cout << "Speed: " << speed_meter_per_sec << '\n';
#endif
							
						}
						
					}
					
					// IF I get a packet indicating a car is less than 20 meters ahead (same lane) AND is going slower than me
					// AND IS in a Platoon
					else if (packet_in.x_position <= x_temp + DANGER_CLOSE // Car ahead is less than DANGER_CLOSE
						&& packet_in.x_position > x_temp // Makes sure this car is ahead, not behind
						&& packet_in.y_position == y_temp // In same lane
						&& packet_in.x_speed <= speed_meter_per_sec // <= so this will keep being executed until LEFT LANE clear
						&& packet_in.platoon_member == true // Car ahead is in a platoon
						&& platoon_member == false) // I am not in a platoon
					{
#ifdef DEBUG_PLATOON
						cout << "--- Platoon: I am in range of a platoon. Sending Request to Join the Platoon.\n";
#endif
						// Create the packet
						tx_packet packet_out;
						
						// Fill the packet with Platoon Joining Info
						packet_out.sequence_num = sequence_counter++;
						packet_out.source_address = my_address;
						packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
						packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
						packet_out.destination_address = 0xFFFFFFFF; // Send to all cars as I don't know the trucks address yet
						packet_out.time_sent = 0;
						packet_out.packet_type = REQUEST_PACKET;
						packet_out.x_position = x_temp;
						packet_out.y_position = y_temp;
						packet_out.x_speed = speed_meter_per_sec;
						packet_out.platoon_member = platoon_member;
						packet_out.number_of_platoon_members = platoon_member_count;
						packet_out.message = REQUEST_ENTER_PLATOON;
						
						// ---- Packet Sending Start ----
						
						int y = 0; // Used to keep track of the Connected hostname / port numbers
						
						// Send the packet to all connected nodes
						for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
						{
							// If we are connected to node i...
							if (connected_nodes[i] == 1)
							{
#ifdef DEBUG_PLATOON
								cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
								cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
								
								// Put the time in the packet right before sending.
								packet_out.time_sent = get_time();
								
								send_packet(nodes[my_node_num].connected_hostnames[y],
											nodes[my_node_num].connected_ports[y],
											packet_out);
								
								// Increase our y counter to prep to send to the next connected node
								y++;
							}
						}
						
						// ---- Packet Sending End ----

						// Match speed of the platoon so I don't run into them, but don't want to pass either
						// Enter a holding pattern
						holding_pattern = true;
						
						// Match car's speed
						speed_meter_per_sec = packet_in.x_speed;
					
					}
					
					// IF I get a packet indicating a car is less than 20 meters ahead (DIFFERENT lane) AND is going slower than me
					// AND IS in a Platoon
					else if (packet_in.x_position <= x_temp + DANGER_CLOSE // Car ahead is less than DANGER_CLOSE
							 && packet_in.x_position > x_temp // Makes sure this car is ahead, not behind
							 && packet_in.y_position != y_temp // In different
							 && packet_in.x_speed <= speed_meter_per_sec // <= so this will keep being executed until LEFT LANE clear
							 && packet_in.platoon_member == true // Car ahead is in a platoon
							 && platoon_member == false) // I am not in a platoon
					{
#ifdef DEBUG_PLATOON
						cout << "--- Platoon: I am in range of a platoon. Sending Request to Join the Platoon.\n";
#endif
						// Create the packet
						tx_packet packet_out;
						
						// Fill the packet with Platoon Joining Info
						packet_out.sequence_num = sequence_counter++;
						packet_out.source_address = my_address;
						packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
						packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
						packet_out.destination_address = 0xFFFFFFFF; // Send to all cars as I don't know the trucks address yet
						packet_out.time_sent = 0;
						packet_out.packet_type = REQUEST_PACKET;
						packet_out.x_position = x_temp;
						packet_out.y_position = y_temp;
						packet_out.x_speed = speed_meter_per_sec;
						packet_out.platoon_member = platoon_member;
						packet_out.number_of_platoon_members = platoon_member_count;
						packet_out.message = REQUEST_ENTER_PLATOON;
						
						// ---- Packet Sending Start ----
						
						int y = 0; // Used to keep track of the Connected hostname / port numbers
						
						// Send the packet to all connected nodes
						for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
						{
							// If we are connected to node i...
							if (connected_nodes[i] == 1)
							{
#ifdef DEBUG_PLATOON
								cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
								cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
								// Put the time in the packet right before sending.
								packet_out.time_sent = get_time();
								
								send_packet(nodes[my_node_num].connected_hostnames[y],
											nodes[my_node_num].connected_ports[y],
											packet_out);
								
								// Increase our y counter to prep to send to the next connected node
								y++;
							}
						}
						
						// ---- Packet Sending End ----
						
						// Match speed of the platoon so I don't run into them, but don't want to pass either
						// Enter a holding pattern
						holding_pattern = true;
						
						// Match car's speed
						speed_meter_per_sec = packet_in.x_speed;
						
						//TODO: CRAZY logic that makes us slow down move into right lane and join the platoon
					}
					
					// Else there is not a car Danger Close ahead and should make sure I am going my starting speed
					// POTENTIAL PROBLEM: This should fix the issue when a car matches the speed of the car ahead, but does so where he stays more than
					// danger close distance away.
					else
					{
#ifdef DEBUG_ROAD_RULES
						//cout << "- ROAD RULES: Nothing Danger Close ahead. -\n";
#endif
						// If I am not in a holding_pattern
						
						// Make sure I am at my starting speed
						//speed_meter_per_sec = starting_speed_meter_per_sec;
					}
					
				} // End IF - Checks if it is a location packet
				
				// Else if we received a Request to Enter a Platoon Packet and I am the Truck
				else if (packet_in.packet_type == REQUEST_PACKET && my_node_num == 1)
				{
					// Some car wants to join our platoon (not the same car asking again, if packet was lost)
					if (packet_in.message == REQUEST_ENTER_PLATOON
						&& packet_in.source_address != address_of_last_car_to_try_and_join_platoon)
					{
#ifdef DEBUG_PLATOON
						cout << "--- Platoon: Truck received request from " << packet_in.source_address;
						cout << " to join the platoon.\n";
#endif
						// Check and see if any other cars are joining
						if (platoon_open)
						{
#ifdef DEBUG_PLATOON
							cout << "--- Platoon: Platoon OPEN.\n";
							cout << "--- Sending Join info.\n";
#endif
							// Increment the car counter
							platoon_member_count++;
							
							// Keep track of what car asked this - Future DEV
							address_of_last_car_to_try_and_join_platoon = packet_in.source_address;
							
							// Send the car his link number (platoon_member_count), wait for All Clear.
							// Create the packet
							tx_packet packet_out;
							
							// Fill the packet with Platoon Joining Info
							packet_out.sequence_num = sequence_counter++;
							packet_out.source_address = my_address;
							packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
							packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
							packet_out.destination_address = packet_in.source_address; // Send to the car who wants to join
							packet_out.time_sent = 0;
							packet_out.packet_type = PLATOON_JOIN_INFO_PACKET;
							packet_out.x_position = x_temp;
							packet_out.y_position = y_temp;
							packet_out.x_speed = speed_meter_per_sec;
							packet_out.platoon_member = platoon_member;
							packet_out.number_of_platoon_members = platoon_member_count;
							packet_out.message = 0;
							
							// ---- Packet Sending Start ----
							
							int y = 0; // Used to keep track of the Connected hostname / port numbers
							
							// Send the packet to all connected nodes
							for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
							{
								// If we are connected to node i...
								if (connected_nodes[i] == 1)
								{
#ifdef DEBUG_PLATOON
									cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
									cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
									// Put the time in the packet right before sending.
									packet_out.time_sent = get_time();
									
									send_packet(nodes[my_node_num].connected_hostnames[y],
												nodes[my_node_num].connected_ports[y],
												packet_out);

									// Increase our y counter to prep to send to the next connected node
									y++;
								}
							}
							
							// ---- Packet Sending End ----
							
							// Set platoon_open to false
							platoon_open = false;
							
							// Wait for all clear then put platoon_open to true and clear address_of_last_car_to_try_and_join_platoon
						}
						
						// Some car is joining and the new car needs to wait and maintain a holding pattern
						else
						{
#ifdef DEBUG_PLATOON
							cout << "--- Platoon: Platoon CLOSED.\n";
							cout << "--- Sending Wait info.\n";
#endif

							// Send a packet to the car telling him to try again
							// Create the packet
							tx_packet packet_out;
							
							// Fill the packet with Platoon Joining Info
							packet_out.sequence_num = sequence_counter++;
							packet_out.source_address = my_address;
							packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
							packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
							packet_out.destination_address = packet_in.source_address; // Send to the car who wants to join
							packet_out.time_sent = 0;
							packet_out.packet_type = PLATOON_WAIT_TO_JOIN_PACKET;
							packet_out.x_position = x_temp;
							packet_out.y_position = y_temp;
							packet_out.x_speed = speed_meter_per_sec;
							packet_out.platoon_member = platoon_member;
							packet_out.number_of_platoon_members = 0;
							packet_out.message = 0;
							
							// ---- Packet Sending Start ----
							
							int y = 0; // Used to keep track of the Connected hostname / port numbers
							
							// Send the packet to all connected nodes
							for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
							{
								// If we are connected to node i...
								if (connected_nodes[i] == 1)
								{
#ifdef DEBUG_PLATOON
									cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
									cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
									// Put the time in the packet right before sending.
									packet_out.time_sent = get_time();
									
									send_packet(nodes[my_node_num].connected_hostnames[y],
												nodes[my_node_num].connected_ports[y],
												packet_out);
									
									// Increase our y counter to prep to send to the next connected node
									y++;
								}
							}
							
							// ---- Packet Sending End ----
						}
					}

					
					else if (packet_in.message == REQUEST_LEAVE_PLATOON)
					{
						// Future Development
					}
				}
				
				// Else if I am a truck and the packet is from a car who has joined the platoon
				else if (packet_in.packet_type == CAR_JOIN_STATUS && my_node_num == 1)
				{
#ifdef DEBUG_PLATOON
					cout << "--- Platoon: Truck received CAR_JOIN_STATUS from " << packet_in.source_address;
					cout << " Status: " << packet_in.message << " (20 = ALL_CLEAR)\n";
#endif
					
					// If the car is done joining
					if (packet_in.message == ALL_CLEAR)
					{
						// Set the Platoon to Open
						platoon_open = true;
					}
					
				}
				
				// Else if I am a car and received a packet from the truck saying I can join and it's for me
				else if (packet_in.packet_type == PLATOON_JOIN_INFO_PACKET
						 && my_node_num != 1
						 && packet_in.destination_address == my_address
						 && platoon_member == false) // And I am not already in the platoon
				{
#ifdef DEBUG_PLATOON
					cout << "--- Platoon: Received confirmation I can join platoon.\n";
#endif
					// Teleport to my platoon position
					x_temp = packet_in.x_position - find_distance_from_truck(packet_in.number_of_platoon_members);
					y_temp = packet_in.y_position;
					
					platoon_member_count = packet_in.number_of_platoon_members;
					platoon_member = true;
					
#ifdef DEBUG_PLATOON
					cout << "--- Platoon: Moving to: " << x_temp << "\n";
#endif
					
					// Match platoon speed
					speed_meter_per_sec = packet_in.x_speed;
					
					// Send ALL_ClEAR packet
					// Send a packet to the car telling him to try again
					// Create the packet
					tx_packet packet_out;
					
					// Fill the packet with Platoon Joining Info
					packet_out.sequence_num = sequence_counter++;
					packet_out.source_address = my_address;
					packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
					packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
					packet_out.destination_address = packet_in.source_address; // Send to the truck
					packet_out.time_sent = 0;
					packet_out.packet_type = CAR_JOIN_STATUS;
					packet_out.x_position = x_temp;
					packet_out.y_position = y_temp;
					packet_out.x_speed = speed_meter_per_sec;
					packet_out.platoon_member = platoon_member;
					packet_out.number_of_platoon_members = 0;
					packet_out.message = ALL_CLEAR;
					
					// ---- Packet Sending Start ----
					
					int y = 0; // Used to keep track of the Connected hostname / port numbers
					
					// Send the packet to all connected nodes
					for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
					{
						// If we are connected to node i...
						if (connected_nodes[i] == 1)
						{
#ifdef DEBUG_PLATOON
							cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
							cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
							// Put the time in the packet right before sending.
							packet_out.time_sent = get_time();
							
							send_packet(nodes[my_node_num].connected_hostnames[y],
										nodes[my_node_num].connected_ports[y],
										packet_out);
							
							// Increase our y counter to prep to send to the next connected node
							y++;
						}
					}
					
					// ---- Packet Sending End ----
				}
				
				// Else if I am a car and received a packet from the truck saying I need to wait
				else if (packet_in.packet_type == PLATOON_WAIT_TO_JOIN_PACKET && my_node_num != 1
						 && packet_in.destination_address == my_address
						 && platoon_member == false)
				{
					// Maybe resend join packet here
					// Maintain speed - should keep asking automagically
		
					speed_meter_per_sec = packet_in.x_speed;
				}
				
				// Record the throughput time (aka Turn Around Time)
				update_throughput_time(get_time() - packet_in.time_sent);
			}
			
#ifdef DEBUG
			cout << "Done servicing popped packet.\n";
			cout << "Buffer Size: " << Buffer.size() << '\n';
#endif
		} // End Popping and processing 1 packet from buffer
		
		// ----- General Road Rules -----
		// These are rules that cars should try to follow even if they are not in range of another car
		// Right now this is really just checking to make sure we don't stay in the LEFT LANE if we start there.
		
		// IF I am in the LEFT LANE, I need to move over if safe
		if (y_temp == LEFT_LANE)
		{
#ifdef DEBUG
			//cout << "ROAD RULES: I am in the LEFT LANE, need to try and get over.\n";
#endif
			// Check to see if right lane is clear (20 meter buffer)
			if (check_if_lane_clear(RIGHT_LANE, nodes, my_node_num))
			{
				// Moving to Right Lane
				y_temp = RIGHT_LANE;
				
#ifdef DEBUG_ROAD_RULES
				cout << "- Road Rules: Moving to RIGHT LANE. -\n";
				//cout << "Buffer Size: " << Buffer.size() << '\n';
				
#endif
				
				// Resume starting speed (may already be at starting speed)
				speed_meter_per_sec = starting_speed_meter_per_sec;
#ifdef DEBUG_ROAD_RULES
				cout << "Speed set to: " << speed_meter_per_sec << '\n';
#endif
			}
			
			// Else not safe to switch lanes
			else
			{
				// If we have not yet increased our speed... increase it. (Makes sure we don't keep going faster and faster)
				// AND we are not in a holding pattern (waiting for some car to move)
				if (speed_meter_per_sec <= starting_speed_meter_per_sec && !holding_pattern)
				{
					// Increase speed by PASSING_SPEED_INCREASE
					speed_meter_per_sec = speed_meter_per_sec + PASSING_SPEED_INCREASE;
#ifdef DEBUG_ROAD_RULES
					cout << "- Passing Speed set to: " << speed_meter_per_sec << " -\n";
#endif
				}
			}
		}

		if (can_transmit)
		{
			// Update our x location (have to do it in here so we can accuratly calculate our delta(x))
			set_new_location(x_temp, speed_meter_per_sec, micro_s);
			
			// Refresh our nodes data structure
			read_node_info(nodes);
			
			// Update nodes with our location (must do after Reading)
			nodes[my_node_num].node_x_coordinate = x_temp;
			nodes[my_node_num].node_y_coordinate = y_temp;
			
			// If we are in a platoon make sure we are where we should be (and not a truck)
			if (platoon_member && my_node_num != 1)
			{
				x_temp = nodes[1].node_x_coordinate - find_distance_from_truck(platoon_member_count);
				nodes[my_node_num].node_x_coordinate = x_temp;
			}
			
#ifdef PROD_OUTPUT
			
			if (my_node_num == 1)
			{
				cout << "Truck:\t";
			}
			
			else
			{
				cout << "Car " << my_node_num - 1 << ":\t";
			}
			
			cout << "GPS: " << x_temp << "\tLane: ";
			
			if (y_temp == LEFT_LANE)
			{
				cout << "Left";
			}
			else if (y_temp == RIGHT_LANE)
			{
				cout << "Right";
			}
			
			cout << "\tPlatoon Member: ";
			
			if (platoon_member)
			{
				cout << "YES";
			}
			else
			{
				cout << "NO";
			}
			
			cout << "\tSpeed: " << speed_meter_per_sec << '\n';
		
		
#endif
		
#ifdef DEBUG_ROAD_RULES
		
			cout << "Current X Location: " << x_temp << '\n';
			cout << "Current Y Location: " << y_temp << '\n';
			cout << "Current Speed: " << speed_meter_per_sec << '\n';
			cout << "Buffer Size: " << Buffer.size() << '\n';
			cout << "Platoon Member?: " << platoon_member << '\n';
			
#endif
			// Use the nodes data structure (from Config.txt) to determine what nodes I can connect to (<100m)
			for (int i = 1; i < MAX_NUM_OF_NODES + 1; i++)
			{
				// Use the Config file to determine what nodes I can connect to (<100m)
				if (((x_temp + 100 >= nodes[i].node_x_coordinate && x_temp <= nodes[i].node_x_coordinate)
					|| (x_temp - 100 <= nodes[i].node_x_coordinate && x_temp >= nodes[i].node_x_coordinate))
					&& (i != my_node_num)) // Makes sure we can't create a link to ourselves.
				{
#ifdef DEBUG
					cout << "In Range of Node: " << i << '\n';
#endif
					
					// If this is a newly found node, need to keep track of it
					if (connected_nodes[i] == 0)
					{
						// Keep track of what nodes we are connected to.
						connected_nodes[i] = 1;
#ifdef DEBUG_ROAD_RULES
						cout << "We have come into range of a new node.\n";
						display_all_connected_nodes(connected_nodes);
//						display_all_node_data(nodes);
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
					//cout << "We are not in range of Node: " << i << '\n';
#endif
					// If we were connected previously, we need to disconnect
					if (connected_nodes[i] == 1)
					{
#ifdef DEBUG_ROAD_RULES
						cout << "We are disconnecting from Node: " << i << '\n';
#endif
						connected_nodes[i] = 0;
						// nodes[my_node_num].number_of_links will tell us how many time we need to loop through the connected hostnames/ports
						// ports will be unique, that is how we will determine which index needs to be removed
						// Then we need to shuffle all the entires so they are left justified in the array... Linked List anyone?
						
#ifdef DEBUG_ROAD_RULES
						cout << "Number of links: " << nodes[my_node_num].number_of_links << '\n';
//						display_all_node_data(nodes);
#endif
						// Remove the links for the host node
						for (int j = 0; j < nodes[my_node_num].number_of_links; j++)
						{
							
							if (nodes[my_node_num].connected_ports[j].compare(nodes[i].node_port_number) == 0)
							{
#ifdef DEBUG_ROAD_RULES
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
						
						// Remove the links for the "remote" node
						for (int j = 0; j < nodes[i].number_of_links; j++)
						{
							
							if (nodes[i].connected_ports[j].compare(nodes[my_node_num].node_port_number) == 0)
							{
#ifdef DEBUG_ROAD_RULES
								cout << "We need to remove index " << j << " from the CLIENT connected_hostnames/ports.\n";
#endif
								
								// Shuffle the rest of the elements over (Overwritting j)
								while (j < nodes[i].number_of_links)
								{
									// Max number of links is 10 (can not have a link to yourself)
									// Therefore connected_hostnames/ports[MAX_NUM_OF_NODES] will always be N/A
									// So we do not need to worry about j+1 pointing out of bounds
									
									nodes[i].connected_hostnames[j] = nodes[i].connected_hostnames[j+1];
									nodes[i].connected_ports[j] = nodes[i].connected_ports[j+1];
									j++;
									
								}
								// Decrement our link counter
								nodes[i].number_of_links--;
								//display_all_node_data(nodes);
								// Break out of FOR loop!!! J is messed up now and will break the FOR loop if you don't.
								break;
							}
						}
						
					}
				}
			}
			
			// At this point our Node has updated all it's links.
			
			// Create the string we are going to put into the config.txt file
			new_line = "Node " + to_string(my_node_num) + " " + nodes[my_node_num].node_hostname
			+ " " + nodes[my_node_num].node_port_number + " " + to_string(x_temp) + " " + to_string(y_temp)
			+ " links";
			
			// Add all the links to the new_line
			for (int i = 0; i < nodes[my_node_num].number_of_links; i++)
			{
				new_line = new_line + " " + nodes[my_node_num].connected_hostnames[i] + " "
				+ nodes[my_node_num].connected_ports[i];
			}
			
			// Terminate with a new line
			new_line = new_line + "\n";
			
#ifdef DEBUG
			cout << new_line;
#endif
			// Create the status packet
			tx_packet packet_out;
			
			// Fill the status packet with Updated Data
			packet_out.sequence_num = sequence_counter++;
			packet_out.source_address = my_address;
			packet_out.previous_hop = my_address;	// Indicate the sender (me) was the last hop
			packet_out.previous_hop_port = atoi(nodes[my_node_num].node_port_number.c_str()); // Convert string to int
			packet_out.destination_address = 0xFFFFFFFF; // All 1's means broadcast
			packet_out.time_sent = 0;
			packet_out.packet_type = LOCATION_PACKET;
			packet_out.x_position = x_temp;
			packet_out.y_position = y_temp;
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
					distance_apart = x_temp - nodes[i].node_x_coordinate;
					
#ifdef DEBUG
					cout << "Distance between nodes " << my_node_num;
					cout << " and " << i << " is " << distance_apart << '\n';
#endif
					
					// Calculate "Packet Loss" probability (If > 100 meters, it can't send)
					if (will_packet_send(distance_apart))
					{
						// If get a green light, we send. Otherwise don't and move on to next neighbor
						
#ifdef DEBUG
						cout << "Sending packet to: " << nodes[my_node_num].connected_hostnames[y];
						cout << " on port: " << nodes[my_node_num].connected_ports[y] << '\n';
#endif
						// Put the time in the packet right before sending.
						packet_out.time_sent = get_time();
						
						send_packet(nodes[my_node_num].connected_hostnames[y],
									nodes[my_node_num].connected_ports[y],
									packet_out);
						
						// Keep track of a successful packet
						packets_sent_count++;
					}
					
					else
					{
						
						packets_lost_count++;
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
			
			// Write your new info (generated form both Config logic and packet buffer logic) to file
			// Config File - Updates Links
			// Packets - Updates Speed, lane position, and platoon forming
			
			// Rewrite the config text file
			rewrite_config_file(new_line);
			
		} // End can_transmit - IF
		
	} // End While
	//		} // End DEBUG IF Can_Transmit
	
	// kill threads
	pthread_exit(NULL);
	
	return 0;
	
}



































