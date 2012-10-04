#ifndef SAL_INCLUDE_COMMON
#define SAL_INCLUDE_COMMON

#ifdef _WIN64
	#define WINDOWS
#elif __unix__
	#define POSIX
#endif

#endif
