#ifndef SAL_INCLUDE_THREAD
#define SAL_INCLUDE_THREAD

#include <C-Utilities/Common.h>
#include "Shared.h"

#ifdef WINDOWS
	typedef unsigned long (__stdcall *SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef void* SAL_Thread;
	typedef void* SAL_Mutex;
	typedef void* SAL_Event;
#elif POSIX
	#include <pthread.h>

	typedef void* (*SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef pthread_t* SAL_Thread;
	
	typedef mutex_t* SAL_Mutex;
	typedef pthread_cond_t* SAL_Event;
#endif

export SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter);
export uint64 SAL_Thread_Join(SAL_Thread thread);
export void SAL_Thread_Terminate(SAL_Thread thread, uint32 exitCode);

export SAL_Mutex SAL_Mutex_Create(void);
export void SAL_Mutex_Free(SAL_Mutex mutex);
export void SAL_Mutex_Acquire(SAL_Mutex mutex);
export void SAL_Mutex_Release(SAL_Mutex mutex);

export SAL_Event SAL_Event_Create(void);
export void SAL_Event_Free(SAL_Event event);
export uint64 SAL_Event_Wait(SAL_Event event);
export void SAL_Event_Signal(SAL_Event event);

#endif
