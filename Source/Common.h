#ifndef SAL_INCLUDE_SHARED
#define SAL_INCLUDE_SHARED

#ifdef _WIN64
  #define WINDOWS
#elif __unix__
  #define POSIX
#endif

#endif
