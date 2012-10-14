#ifndef INCLUDE_SAL_THREAD
#define INCLUDE_SAL_THREAD

#include "Common.h"

#ifdef WINDOWS
	#define SAL_Thread_Start(Name) unsigned long __stdcall Name(void* startupArgument)
	typedef unsigned long (__stdcall *SAL_Thread_StartAddress)(void* SAL_Thread_StartArgument);
	typedef void* SAL_Thread;
	typedef void* SAL_Mutex;
	typedef void* SAL_Semaphore;
#elif defined POSIX
	#include <pthread.h>
  #include <semaphore.h>
    
	#define SAL_Thread_Start(Name) void* ##Name(void* startupArgument) {
	typedef void* (*SAL_Thread_StartAddress)(void* SAL_Thread_StartArgument);
	typedef pthread_t SAL_Thread;
	
	typedef pthread_mutex_t* SAL_Mutex;
    typedef sem_t* SAL_Semaphore;
#endif

public SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter);
public uint64 SAL_Thread_Join(SAL_Thread thread);
public void SAL_Thread_Yield(void);
public void SAL_Thread_Sleep(uint32 duration);
public void SAL_Thread_Exit(uint32 exitCode);

public SAL_Mutex SAL_Mutex_Create(void);
public uint8 SAL_Mutex_Free(SAL_Mutex mutex);
public void SAL_Mutex_Acquire(SAL_Mutex mutex);
public void SAL_Mutex_Release(SAL_Mutex mutex);

public SAL_Semaphore SAL_Semaphore_Create(void);
public void SAL_Semaphore_Free(SAL_Semaphore Semaphore);
public void SAL_Semaphore_Decrement(SAL_Semaphore Semaphore);
public void SAL_Semaphore_Increment(SAL_Semaphore Semaphore);

#endif
