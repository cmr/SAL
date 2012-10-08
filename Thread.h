#ifndef SAL_INCLUDE_THREAD
#define SAL_INCLUDE_THREAD

#include <C-Utilities/Common.h>
#include "Common.h"

#ifdef WINDOWS
	typedef unsigned long (__stdcall *SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef void* SAL_Thread;
	typedef void* SAL_Mutex;
	typedef void* SAL_Event;
#elif defined POSIX
	#include <pthread.h>
	#include <errno.h>

	typedef void* (*SAL_Thread_StartAddress)(void* SAL_Thread_StartParameter);
	typedef pthread_t SAL_Thread;
	
	typedef pthread_mutex_t* SAL_Mutex;
	typedef pthread_cond_t* SAL_Event;
#endif

public SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter);
public uint64 SAL_Thread_Join(SAL_Thread thread);
public void SAL_Thread_Exit(uint32 exitCode);
public void SAL_Thread_Terminate(SAL_Thread thread, uint32 exitCode);

public SAL_Mutex SAL_Mutex_Create(void);
public uint8 SAL_Mutex_Free(SAL_Mutex mutex);
public void SAL_Mutex_Acquire(SAL_Mutex mutex);
public void SAL_Mutex_Release(SAL_Mutex mutex);

public SAL_Event SAL_Event_Create(void);
public void SAL_Event_Free(SAL_Event event);
public uint64 SAL_Event_Wait(SAL_Event event);
public void SAL_Event_Signal(SAL_Event event);

#endif
