#include "Time.h"

#ifdef WINDOWS
	#include <Windows.h>
#endif

uint64 SAL_Time_Now() {
	#ifdef WINDOWS
		FILETIME time;
		uint64 result;
	 
		GetSystemTimeAsFileTime(&time);

		result = time.dwHighDateTime;
		result <<= 32;
		result += time.dwLowDateTime;
		result /= 10000; //to shift from 100ns intervals to 1ms intervals
		result -= 12969849600000; //to shift from epoch of 1/1/1601 to 1/1/2012

		return result;
	#elif POSIX
		
	#endif
}