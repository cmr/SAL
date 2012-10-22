#ifndef INCLUDE_SAL_SOCKET
#define INCLUDE_SAL_SOCKET

#include "Common.h"
#include <Utilities/Strings.h>

#ifdef WINDOWS
	typedef uint64* SAL_Socket;
#elif defined POSIX
	typedef void* SAL_Socket;
#endif

typedef void (*SAL_Socket_ReadCallback)(const uint8* const buffer, const uint32 length, void* const state);

public SAL_Socket SAL_Socket_Connect(const int8* const address, const uint16 port);
public SAL_Socket SAL_Socket_ConnectIP(const uint32 ip, const uint16 port);
public SAL_Socket SAL_Socket_Listen(const int8* const port);
public SAL_Socket SAL_Socket_Accept(SAL_Socket listener, uint32* const acceptedAddress);
public void SAL_Socket_Close(SAL_Socket socket);
public uint32 SAL_Socket_Read(SAL_Socket socket, uint8* const buffer, const uint32 bufferSize);
public boolean SAL_Socket_Write(SAL_Socket socket, const uint8* const toWrite, const uint32 writeAmount);
public void SAL_Socket_RegisterReadCallback(SAL_Socket socket, SAL_Socket_ReadCallback callback, void* const state);
public void SAL_Socket_UnregisterSocketCallbacks(SAL_Socket socket);

#endif
