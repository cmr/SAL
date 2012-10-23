/** vim: set noet ci pi sts=0 sw=4 ts=4
 * @file Time.c
 * @brief Functions for getting the current time.
 */

#include "Time.h"

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#elif defined POSIX
	#include <sys/time.h>
#endif

/**
 * @returns the current time in ms since Jan 1, 1970.
 *
 * @warning Does *NOT* take into account leap seconds, but does take into
 * account leap days.
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

	gettimeofday(&tv, NULL);

	result = tv.tv_sec;
	result *= 1000000; // convert seconds to microseconds
	result += tv.tv_usec; // add microseconds
	result /= 1000; // convert to milliseconds
	return result;
#endif
}
