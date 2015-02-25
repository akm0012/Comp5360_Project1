#include <iostream>
#include <cstdlib>
#include <pthread.h>
#include <deque>
#include <time.h>
#include <unistd.h>
#include <string>

using namespace std;

#define NUM_THREADS     1

deque<int> Buffer;

// function to thread
void *PrintHello(void *threadid)
{
	// thread id
	long tID;
	tID = (long)threadid;
	cout << "Hello World! Thread ID, " << tID << endl;
	while(1)
	{
		// push a random number from 1 to 20 onto the Buffer
		Buffer.push_back(rand() % 20 + 1);
		// wait for readability
		sleep(1);
	}
	// exit thread
	pthread_exit(NULL);
}

int main ()
{
	// seed the RNG
	srand(time(NULL));

	// define how many threads we're going to use
	pthread_t threads[NUM_THREADS];
	int rc;
	for(int i=0; i < NUM_THREADS; i++ )
	{
		cout << "main() : creating thread, " << i << endl;
		// create threads
		rc = pthread_create(&threads[i], NULL, PrintHello, (void *)i);
		// check for creation errors
		if (rc)
		{
			cout << "Error: unable to create thread," << rc << endl;
			exit(-1);
		}
	}
	
	while (1)
	{
		// if the Buffer isn't empty
		if (!Buffer.empty())
		{
			// print the first element
			cout << to_string(Buffer.front()) << endl;
			// delete the first element
			Buffer.pop_front();
		}
	}
	
	// kill threads
	pthread_exit(NULL);
}
