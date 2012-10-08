/** @file Thread.c
 * @brief Threading functions and synchronization primitives.
 */

#include "Thread.h"

#ifdef WINDOWS
	#include <Windows.h>
#endif

/**
 * Create a thread.
 *
 * @param startAddress The function the thread will execute
 * @param startParameter The parameter the function will be called with
 */
SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter) {
	#ifdef WINDOWS
		return CreateThread(null, 0, startAddress, startParameter, 0, null);
	#elif defined POSIX
		pthread_t threadId;

		pthread_create(&threadId, null, startAddress, startParameter);

		return threadId;
	#endif
}

/**
 * Wait for @a thread to exit.
 *
 * @returns 0 on success, non-0 on failure.
 */
uint64 SAL_Thread_Join(SAL_Thread thread) {
	#ifdef WINDOWS
		DWORD result;

		result = WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);

		return (uint64)result;
	#elif defined POSIX
		return (uint64)pthread_join(thread, null);
	#endif
}

/**
 * Exit the current running thread.
 *
 * @param exitCode Return code of the thread
 */
void SAL_Thread_Exit(uint32 exitCode) {
	#ifdef WINDOWS
		ExitThread(exitCode);
	#elif defined POSIX
		pthread_exit((void*)&exitCode);
	#endif
}

/**
 * Terminate the current running thread.
 *
 * @param exitCode Return code of the thread
 * @param thread Thread to forcibly terminate
 */
void SAL_Thread_Terminate(SAL_Thread* thread, uint32 exitCode) {
	#ifdef WINDOWS
		TerminateThread((HANDLE)thread, exitCode);
	#elif defined POSIX
		pthread_exit((void*)&exitCode); //need to make this a proper termination
	#endif
}

/**
 * Create a new mutex.
 *
 * The created mutex should be freed with @ref SAL_Mutex_Free as to not leak
 * memory. Do not free it with @a free.
 *
 * @returns a new mutex
 */
SAL_Mutex SAL_Mutex_Create(void) {
	#ifdef WINDOWS
		CRITICAL_SECTION* criticalSection;

		criticalSection = Allocate(CRITICAL_SECTION);
		InitializeCriticalSection(criticalSection);

		return (SAL_Mutex)criticalSection;
	#elif defined POSIX
		pthread_mutex_t *mutex = Allocate(pthread_mutex_t);
		pthread_mutex_init(mutex, null);
		return mutex;
	#endif
}

/**
 * Frees @a mutex.
 *
 * @param mutex mutex to free
 *
 * @returns 0 if the mutex is sucessfully freed, 1 if the mutex is still being
 * used.
 *
 * @warning On windows, you can free a mutex that is still being used. Be
 * careful not to free mutexes that are still in use. Only on POSIX will you
 * have a safety net.
 */
uint8 SAL_Mutex_Free(SAL_Mutex mutex) {
	#ifdef WINDOWS
		DeleteCriticalSection((CRITICAL_SECTION*)mutex);
		return 0;
	#elif defined POSIX
		int status;
		status = pthread_mutex_destroy(mutex);
		if (status == EBUSY) {
		  return 1;
		}
		return 0;
	#endif
}

/**
 * Acquire a lock.
 * 
 * @param mutex mutex to lock.
 *
 * @warning This function will block until the mutex is acquired.
 */
void SAL_Mutex_Acquire(SAL_Mutex mutex) {
	#ifdef WINDOWS
		EnterCriticalSection((CRITICAL_SECTION*)mutex);
	#elif defined POSIX
		pthread_mutex_lock(mutex);
	#endif
}

/**
 * Release a lock.
 *
 * @param mutex mutex to unlock
 */
void SAL_Mutex_Release(SAL_Mutex mutex) {
	#ifdef WINDOWS
		LeaveCriticalSection((CRITICAL_SECTION*)mutex);
	#elif defined POSIX
		pthread_mutex_unlock(mutex);
	#endif
}

/**
 * Create a new event.
 *
 * @return a new event
 */
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
