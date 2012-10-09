#ifndef SAL_INCLUDE_SOCKET
#define SAL_INCLUDE_SOCKET

#include "Common.h"

#ifdef WINDOWS
	typedef void* SAL_Socket;
#elif defined POSIX
    typedef void* SAL_Socket;
#endif

public void SAL_Socket_Free(SAL_Socket socket);
public SAL_Socket SAL_Socket_Connect(uint8* address);
public SAL_Socket SAL_Socket_Bind(uint8* address);
public void SAL_Socket_Listen(SAL_Socket socket);
public SAL_Socket SAL_Socket_Accept(SAL_Socket socket);
public void SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint64 bufferSize, uint64 readAmount);
public void SAL_Socket_Write(SAL_Socket socket, uint8* toWrite, uint64 writeAmount);

#endif
