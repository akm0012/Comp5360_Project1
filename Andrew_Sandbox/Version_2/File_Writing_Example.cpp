//
//  File_Writing_Example.cpp
//  
//
//  Created by Andrew Marshall on 2/23/15.
//
//

#include "File_Writing_Example.h"
#include <iostream>
#include <fstream>
#include <unistd.h>

#define DEBUG 1



using namespace std;

struct node_info
{
	// Will prob need a Truck / Car identifer
	int node_number;
	string node_hostname;
	string node_port_number;
	int node_x_coordinate;
	int node_y_coordinate;
	
	int number_of_links;
	
	string connected_hostnames[10];
	string connected_ports[10];
};




// Ptorotypes
//void read_node_info(node_info (&nodes)[11]);


/* Functions */

/*
 *
 *
 */
void read_node_info(node_info (&nodes)[11]) {
	
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
				
				cout << "\nNode number: " << nodes[node_num].node_number << '\n';
				cout << "Host Name: " << nodes[node_num].node_hostname << '\n';
				cout << "Host Port: " << nodes[node_num].node_port_number << '\n';
				cout << "X Coordinate: " << nodes[node_num].node_x_coordinate << '\n';
				cout << "Y Coordinate: " << nodes[node_num].node_y_coordinate << '\n';
				
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
	ifstream ifile(filename);
	return ifile;
}

/*
 *
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
		outStream << "-1 -1 " << "links";
		
		outStream.close();
		
		// Indicate that this is node 1
		node_number = 1;
	}
	
	
	return node_number;
}





int main () {
	
	// NOTE: For ease of use the index will match the Node Number
	// So nodes [0] will remain empty.
	int my_node_num;
	node_info nodes[11];
	
	string hostname = "test_hostname";
	string port_num = "test_port_num";
	
	my_node_num = initial_config(hostname, port_num);
	
	if (my_node_num == 1)
	{
		// I am lead truck... do stuff here
	}
	
	else
	{
		// Read in node info to determine starting X and Y coord.
	}
	
	if (DEBUG) {
		cout << "Node Number: " << my_node_num << '\n';
	}
	
	read_node_info(nodes);
	
//	while (1)
//	{
//		sleep(1);
//		read_node_info(nodes);
//		
//	}
	
	
	return 0;
	
}



































