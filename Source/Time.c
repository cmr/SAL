#ifdef WINDOWS
	#include <Windows.h>
#elif POSIX
  #include <sys/time.h>
#endif

#include "Time.h"

/**
 * Return the current time in ms since Jan 1, 1970.
 *
 * Does *NOT* take into account leap seconds, but does take into account leap
 * days.
 */
int64 SAL_Time_Now(void) {
	#ifdef WINDOWS
		FILETIME time;
		uint64 result;
	 
		GetSystemTimeAsFileTime(&time);

		result = time.dwHighDateTime;
		result <<= 32;
		result += time.dwLowDateTime;
		result /= 10000; // to shift from 100ns intervals to 1ms intervals
		result -= 11644473600000; // to shift from epoch of 1/1/1601 to 1/1/1970

		return (int64)result;
	#elif defined POSIX
		int64 result;
    struct timeval tv;

    gettimeofday(&tv, null);

    result = tv.tv_sec;
    result *= 1000000; // convert seconds to microseconds
    result += tv.tv_usec; // add microseconds
    result /= 1000; // convert to milliseconds
    return result;
	#endif
}
