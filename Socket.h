#ifndef INCLUDE_SAL_SOCKET
#define INCLUDE_SAL_SOCKET

#include "Common.h"

/* forward declaration */
typedef struct SAL_Socket SAL_Socket;

typedef void (*SAL_Socket_ReadCallback)(SAL_Socket* socket, void* const state);

#define SAL_Socket_Families_IPV4 0
#define SAL_Socket_Families_IPV6 1
#define SAL_Socket_Families_IPAny 2

#define SAL_Socket_Types_TCP 0 /* should probably add UDP eventually */

#define SAL_Socket_AddressLength 16

struct SAL_Socket {
	#ifdef WINDOWS
		uint64 RawSocket;
	#elif defined POSIX
		int RawSocket;
	#endif
	uint8 Type;
	uint8 Family;
	boolean Connected;
	uint8 LastError;
	uint8 RemoteEndpointAddress[SAL_Socket_AddressLength];
	SAL_Socket_ReadCallback ReadCallback;
	void* ReadCallbackState;
};

public SAL_Socket* SAL_Socket_Connect(const int8* const address, const int8* port, uint8 family, uint8 type);
public SAL_Socket* SAL_Socket_Listen(const int8* const port, uint8 family, uint8 type);
public SAL_Socket* SAL_Socket_Accept(SAL_Socket* listener);
public void SAL_Socket_Close(SAL_Socket* socket);
public uint32 SAL_Socket_Read(SAL_Socket* socket, uint8* const buffer, const uint32 bufferSize);
public uint32 SAL_Socket_Write(SAL_Socket* socket, const uint8* const toWrite, const uint32 writeAmount);
public void SAL_Socket_SetReadCallback(SAL_Socket* socket, SAL_Socket_ReadCallback callback, void* const state);
public void SAL_Socket_UnsetSocketCallback(SAL_Socket* socket);

#endif
