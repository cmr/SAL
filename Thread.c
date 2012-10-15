/** vim: set noet ci pi sts=0 sw=4 ts=4
 * @file Thread.c
 * @brief Threading functions and synchronization primitives.
 */

#include "Thread.h"

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
#elif defined POSIX
	#include <errno.h>
#endif

/**
 * Create a thread.
 *
 * @param startAddress The function the thread will execute
 * @param startParameter The parameter the function will be called with
 *
 * @returns An opaque thread id object.
 */
SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startArgument) {
#ifdef WINDOWS
	return CreateThread(NULL, 0, startAddress, startArgument, 0, NULL);
#elif defined POSIX
	pthread_t threadId;

	pthread_create(&threadId, NULL, startAddress, startArgument);

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
	return (uint64)pthread_join(thread, NULL);
#endif
}

/**
 * Yields execution of @a thread, allowing another thread to execute.
 */
void SAL_Thread_Yield(void) {
#ifdef WINDOWS
	Sleep(0);
#elif defined POSIX
	pthread_yield();
#endif
}

/**
 * Suspend execution of current thread.
 *
 * @param duration Length of time to sleep, in milliseconds
 */
void SAL_Thread_Sleep(uint32 duration) {
#ifdef WINDOWS
	Sleep(duration);
#elif defined POSIX
	sleep(duration);
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
	pthread_mutex_init(mutex, NULL);
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
 * Create a new semaphore.
 *
 * @returns a new semaphore, NULL on failure
 */
SAL_Semaphore SAL_Semaphore_Create(void) {
#ifdef WINDOWS
	return CreateSemaphore(NULL, 0, 4294967295, NULL);
#elif defined POSIX
	sem_t *sem = Allocate(sem_t);
	if (sem_init(sem, 0 /* shared between threads */, 0) != 0) {
		return NULL;
	}
	return sem;
#endif
}

/**
 * Free a semaphore.
 * 
 * @param semaphore to free
 * @warning Don't free a semaphor that is still in use (being waited on)
 */
void SAL_Semaphore_Free(SAL_Semaphore semaphore) {
#ifdef WINDOWS
	CloseHandle(semaphore);
#elif defined POSIX
	sem_destroy(semaphore);
	Free(semaphore);
#endif
}

/**
 * Decrement a semaphore.
 * 
 * @param semaphore to decrement
 *
 * @warning This function will block if the semaphore count is zero.
 */
void SAL_Semaphore_Decrement(SAL_Semaphore semaphore) {
#ifdef WINDOWS
	WaitForSingleObject(semaphore, INFINITE);
#elif defined POSIX
	while (sem_wait(semaphore) == -1 && errno == EINTR);
#endif
}

/**
 * Increment a semaphore.
 * 
 * @param semaphore to increment
 */
void SAL_Semaphore_Increment(SAL_Semaphore semaphore) {
#ifdef WINDOWS
	ReleaseSemaphore(semaphore, 1, NULL);
#elif defined POSIX
	sem_post(semaphore);
#endif
}
