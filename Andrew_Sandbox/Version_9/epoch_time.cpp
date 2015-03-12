/* Evan Hall 3/10/2015
 *
 * I NEED THESE COMPILATION FLAGS:
 *
 *			-lm
 */

#include <inttypes.h>
#include <math.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <string>
#include <iomanip>

using namespace std;

typedef struct {
	time_t sec;
	long nsec;
} utime;

utime get_time_from_epoch() {
	utime result;
	struct timespec spec;
	
	clock_gettime(CLOCK_REALTIME, &spec);
	
	result.sec = spec.tv_sec;
	result.nsec = spec.tv_nsec;
	
	return result;
}

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

int main() {
	
	string number;
	double result;
	double test;
	utime now;
	
	while (1) {
		
		now = get_time_from_epoch();
		
		//printf("%"PRIdMAX"\ts\t%09ld\tns\n", (intmax_t)now.sec, now.nsec);
		
		///////
		
		number = "";
		number.append(to_string(now.sec));
		number = number.substr(6, number.length());
		number.append(".");
		number.append(to_string(now.nsec));
		
		result = stod(number);
		////////
		
		//printf("%s\n", number.c_str());
		
		test = get_time();
		
		cout << setprecision(14) << test << endl;
		
		//usleep(100000);
	}
	
	return 0;
}
