#ifndef SAL_INCLUDE_THREAD
#define SAL_INCLUDE_THREAD

#include "Shared.h"
#include <C-Utilities/Source/Common.h>

#ifdef WINDOWS
	typedef unsigned long (__stdcall *SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef void* SAL_Thread;
#elif POSIX
	#include <pthread.h>

	typedef void* (*SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef pthread_t SAL_Thread;
#endif

export SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter);
export uint64 SAL_Thread_Join(SAL_Thread thread, uint64 timeout);
export void SAL_Thread_Terminate(SAL_Thread thread, uint64 exitCode);

#endif
