#ifndef INCLUDE_SAL_COMMON
#define INCLUDE_SAL_COMMON

#ifdef _WIN64
  #define WINDOWS
#elif __unix__
  #define POSIX
#endif

#include <Utilities/Common.h>

#endif
