#ifndef INCLUDE_SAL_SOCKET
#define INCLUDE_SAL_SOCKET

#include "Common.h"
#include <Utilities/Strings.h>

#ifdef WINDOWS
	typedef void* SAL_Socket;
#elif defined POSIX
    typedef void* SAL_Socket;
#endif

typedef void (*SAL_Socket_ReadCallback)(uint64 bytesRead);

public SAL_Socket SAL_Socket_Connect(uint8* address);
public SAL_Socket SAL_Socket_Listen(uint8* address);
public SAL_Socket SAL_Socket_Accept(SAL_Socket socket, String* acceptedAddress);
public void SAL_Socket_Close(SAL_Socket socket);
public uint64 SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint64 bufferSize);
public uint64 SAL_Socket_ReadAsync(SAL_Socket socket, uint8* buffer, uint64 bufferSize, SAL_Socket_ReadCallback callback);
public void SAL_Socket_Write(SAL_Socket socket, uint8* toWrite, uint64 writeAmount);

#endif
