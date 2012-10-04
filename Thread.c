#include "Thread.h"

#ifdef WINDOWS
	#include <Windows.h>
#endif

SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter) {
	#ifdef WINDOWS
		return CreateThread(null, 0, startAddress, startParameter, 0, null);
	#elif defined POSIX
		pthread_t threadId;

		pthread_create(&threadId, null, startAddress, startParameter);

		return &threadId;
	#endif
}

uint64 SAL_Thread_Join(SAL_Thread thread) {
	#ifdef WINDOWS
		DWORD result;

		result = WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);

		return result;
	#elif defined POSIX
		return pthread_join(thread, null);
	#endif
}

void SAL_Thread_Terminate(SAL_Thread thread, uint32 exitCode) {
	#ifdef WINDOWS
		TerminateThread(thread, exitCode);
		CloseHandle(thread);
	#elif defined POSIX
		pthread_exit((void*)&exitCode);
	#endif
}

SAL_Mutex SAL_Mutex_Create(void) {
	#ifdef WINDOWS
		CRITICAL_SECTION* criticalSection;

		criticalSection = Allocate(CRITICAL_SECTION);
		InitializeCriticalSection(criticalSection);

		return (SAL_Mutex)criticalSection;
	#elif defined POSIX

	#endif
}

void SAL_Mutex_Free(SAL_Mutex mutex) {
	#ifdef WINDOWS
		DeleteCriticalSection((CRITICAL_SECTION*)mutex);
	#elif defined POSIX

	#endif
}

void SAL_Mutex_Acquire(SAL_Mutex mutex) {
	#ifdef WINDOWS
		EnterCriticalSection((CRITICAL_SECTION*)mutex);
	#elif defined POSIX

	#endif
}

void SAL_Mutex_Release(SAL_Mutex mutex) {
	#ifdef WINDOWS
		LeaveCriticalSection((CRITICAL_SECTION*)mutex);
	#elif defined POSIX

	#endif
}

SAL_Event SAL_Event_Create(void) {
	#ifdef WINDOWS
		return CreateEvent(null, false, false, null);
	#elif defined POSIX

	#endif
}

void SAL_Event_Free(SAL_Event event) {
	#ifdef WINDOWS
		CloseHandle(event);
	#elif defined POSIX

	#endif
}

uint64 SAL_Event_Wait(SAL_Event event) {
	#ifdef WINDOWS
		DWORD result;
		
		result = WaitForSingleObject(event, INFINITE);

		return result;
	#elif defined POSIX

	#endif
}

void SAL_Event_Signal(SAL_Event event) {
	#ifdef WINDOWS
		SetEvent(event);
	#elif defined POSIX

	#endif
}
