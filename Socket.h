#ifndef INCLUDE_SAL_SOCKET
#define INCLUDE_SAL_SOCKET

#include "Common.h"
#include <Utilities/Strings.h>

#ifdef WINDOWS
	typedef uint64 SAL_Socket;
#elif defined POSIX
	typedef void* SAL_Socket;
#endif

typedef void (*SAL_Socket_ReadCallback)(uint8* buffer, uint32 length);
#define SAL_Socket_ReadCallback(Name) void Name(uint8* buffer, uint32 length)

public SAL_Socket SAL_Socket_Connect(const int8* address, uint16 port);
public SAL_Socket SAL_Socket_ConnectIP(uint32 ip, uint16 port);
public SAL_Socket SAL_Socket_Listen(const int8* port);
public SAL_Socket SAL_Socket_Accept(SAL_Socket listener, uint32* acceptedAddress);
public void SAL_Socket_Close(SAL_Socket socket);
public uint32 SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint32 bufferSize);
public boolean SAL_Socket_Write(SAL_Socket socket, const uint8* toWrite, uint32 writeAmount);
public void SAL_Socket_RegisterReadCallback(SAL_Socket socket, SAL_Socket_ReadCallback callback);

#endif
