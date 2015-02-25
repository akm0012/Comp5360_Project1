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

#define DEBUG 0

using namespace std;

struct node_info
{
	// Will prob need a Truck / Car identifer
	int node_number;
	string node_hostname;
	int node_port_number;
	int node_x_coordinate;
	int node_y_coordinate;
	
	int number_of_links;
	
	string connected_hostnames[10];
	int connected_ports[10];
};

int main () {

	// NOTE: For ease of use the index will match the Node Number
	// So nodes [0] will remain empty.
	node_info nodes[10];
	
	ifstream myfile;
	
	string word;
	
	int node_num;

	int link_counter;
	string next_word;

	
	myfile.open("Config.txt");

	if (myfile.is_open())
	{
		while (myfile >> word) // Returns false if it can not read from file
		{
			if (word.compare("Node") == 0)
			{
				if (DEBUG) {
					cout << "Debug: NODE KEYWORD FOUND\n";
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
							cout << "Number of Links found: " << nodes[node_num].number_of_links << '\n';
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
		cout << "Unable to open file.\n";
	}
	
	return 0;
}


