/* Evan Hall 3/10/2015
 *
 * I NEED THESE COMPILATION FLAGS:
 *
 * TODO: add this flag during compilation
 *			-lm
 */

#include <inttypes.h>	// needed for PRI%MAX
						//					where PRIdMAX is the equivalent of d
						//						(in "%d") for intmax_t values
						//			  intmax_t
						//					where intmax_t is an int type of maximum
						//						supported width
						//
#include <math.h>		// needed for round(long double)
						//			  1.0e6
						//
#include <stdio.h>		// needed for printf(...)
						//
#include <time.h>		// needed for time_t
						//			  timespec
						//			  clock_gettime(clockid_t, struct timespec *)
						//			  CLOCK_REALTIME

typedef struct {
	time_t sec;
	long msec;
} utime;

utime get_time_from_epoch()
{
	utime result;
	struct timespec spec;
	
	clock_gettime(CLOCK_REALTIME, &spec);
	
	result.sec = spec.tv_sec;
	result.msec = round(spec.tv_nsec / 10e6);
	
	return result;
}

int main() {

	while (1) {
	
		utime now = get_time_from_epoch();

		//	*a printf flag defined outside the printf header
		//	#change the decimal precision here (it's 3 here)
		//
		//					  /--  *  --/ /#/
		printf("Current time: %"PRIdMAX".%03ld seconds since Epoch\n",
			   (intmax_t)now.sec, now.msec);
		
		
	}

	return 0;
}

