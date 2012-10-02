#include "Thread.h"

#ifdef WINDOWS
	#include <Windows.h>
#endif

SAL_Thread SAL_Thread_Create(SAL_Thread_StartAddress startAddress, void* startParameter) {
	#ifdef WINDOWS
		return CreateThread(null, 0, startAddress, startParameter, null, null);
	#elif POSIX
		pthread_t threadId;

		pthread_create(&threadId, null, startAddress, startParameter);

		return threadId;
	#endif
}

uint64 SAL_Thread_Join(SAL_Thread thread) {
	#ifdef WINDOWS
		DWORD result;

		result = WaitForSingleObject(thread, INFINITE);
		CloseHandle(thread);

		return result;
	#elif POSIX
		return pthread_join(thread, null);
	#endif
}

void SAL_Thread_Terminate(SAL_Thread thread, uint64 exitCode) {
	#ifdef WINDOWS
		TerminateThread(thread, exitCode);
		CloseHandle(thread);
	#elif POSIX
		pthread_exit((void*)&exitCode);
	#endif
}