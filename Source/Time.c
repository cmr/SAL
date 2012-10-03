#include "Time.h"

#ifdef WINDOWS
	#include <Windows.h>
#elif POSIX
  #include <sys/time.h>
#endif

/* Return the current time in ms since January 1, 2012 */
uint64 SAL_Time_Now() {
	#ifdef WINDOWS
		FILETIME time;
		uint64 result;
	 
		GetSystemTimeAsFileTime(&time);

		result = time.dwHighDateTime;
		result <<= 32;
		result += time.dwLowDateTime;
		result /= 10000; // to shift from 100ns intervals to 1ms intervals
		result -= 12969849600000; // to shift from epoch of 1/1/1601 to 1/1/2012

		return result;
	#elif POSIX
		int64 result;
    struct timeval tv;

    gettimeofday(&tv, null);

    result = time.tv_sec;
    result *= 1000000; // convert seconds to microseconds
    result += time.tv_usec; // add microseconds
    result /= 1000; // convert to milliseconds
    result -= 1325376000000; // shift from epoch of Jan 1 1970 to Jan 1 2012
    if (result < 0) {
      // what do we do in this case?
    }
    return (uint64)result;
	#endif
}
