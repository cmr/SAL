/** vim: set noet ci sts=0 sw=4 ts=4
 * @file Socket.c
 * @brief TCP networking functions
 *
 * @warning Under windows, only IPv4 is implemented.
 * Under POSIX, IPv4 and IPv6 are supported.
 */
#include "Socket.h"

#include <Utilities/AsyncLinkedList.h>
#include <Utilities/Memory.h>
#include "Thread.h"

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define FD_SETSIZE 1024
	#include <Windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>

	static boolean winsockInitialized = false;
#elif defined POSIX
	#include <sys/select.h>
	#include <sys/socket.h>
	#include <sys/types.h>
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <netdb.h>
	#include <stdio.h>
	#include <string.h>
#endif

static void SAL_Socket_Initialize(SAL_Socket* socket);
static void SAL_Socket_CallbackWorker_Initialize();
static void SAL_Socket_CallbackWorker_Shutdown();
static SAL_Thread_Start(SAL_Socket_CallbackWorker_Run);

static AsyncLinkedList asyncSocketList;
static SAL_Thread asyncWorker;
static boolean asyncWorkerRunning = false;

static SAL_Thread_Start(SAL_Socket_CallbackWorker_Run) {
#ifdef WINDOWS
	fd_set readSet;
	uint32 i;
	SAL_Socket* asyncSocket;
	AsyncLinkedList_Iterator* selectIterator;
	struct timeval selectTimeout;

	selectIterator = AsyncLinkedList_BeginIterate(&asyncSocketList);
	selectTimeout.tv_usec = 250;
	selectTimeout.tv_sec = 0;

	while (asyncWorkerRunning) {
		FD_ZERO(&readSet);

		/* iterates over all sockets with registered callbacks. It either finishes when 1024 sockets have been added or the socket list is exhausted. If the socket list is greater than 1024, the position is remembered on the next loop   */
		for (i = 0; i < FD_SETSIZE && AsyncLinkedList_IterateNext(asyncSocket, selectIterator, SAL_Socket*); i++) {
			#ifdef WINDOWS
				FD_SET((SOCKET)asyncSocket->RawSocket, &readSet);
			#else defined POSIX

			#endif
		}

		if (asyncSocket == NULL)
			AsyncLinkedList_ResetIterator(selectIterator);

		select(0, &readSet, NULL, NULL, &selectTimeout);

		for (i = 0; i < readSet.fd_count; i++) {
			AsyncLinkedList_ForEach(asyncSocket, &asyncSocketList, SAL_Socket*) {
				if (asyncSocket->RawSocket == readSet.fd_array[i]) {
					asyncSocket->ReadCallback(asyncSocket, asyncSocket->ReadCallbackState);
				}
			}
		}

		SAL_Thread_Sleep(25);
	}

	AsyncLinkedList_EndIterate(selectIterator);

	return 0;
#elif defined POSIX

#endif
}

static void SAL_Socket_CallbackWorker_Initialize() {
	AsyncLinkedList_Initialize(&asyncSocketList, Memory_Free);
	asyncWorkerRunning = true;
	asyncWorker = SAL_Thread_Create(SAL_Socket_CallbackWorker_Run, NULL);
}

static void SAL_Socket_CallbackWorker_Shutdown() {
	asyncWorkerRunning = false;
	SAL_Thread_Join(asyncWorker);
	AsyncLinkedList_Uninitialize(&asyncSocketList);
}

static void SAL_Socket_Initialize(SAL_Socket* socket) {
	socket->RawSocket = 0;
	socket->Connected = false;
	socket->LastError = 0;
	socket->ReadCallback = NULL;
	socket->ReadCallbackState = NULL;
	return;
}


/**
 * Create a TCP connection to a host.
 *
 * @param address A string specifying the hostname to connect to
 * @param port Port to connect to
 */
SAL_Socket* SAL_Socket_Connect(const int8* const address, const int8* port) {
#ifdef WINDOWS
	SAL_Socket *sock;
	SOCKET sock_fd;
	struct addrinfo *server, hints;

	if (!winsockInitialized) {
		WSADATA startupData;
		WSAStartup(514, &startupData);
		winsockInitialized = true;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // IPvWhatever
	hints.ai_socktype = SOCK_STREAM; // TCP
	
	if (getaddrinfo(address, port, &hints, &server) != 0) {
		return NULL;
	}

	sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	if (connect(sock_fd, server->ai_addr, (int)server->ai_addrlen) == -1) {
		freeaddrinfo(server);
		return NULL;
	}

	freeaddrinfo(server);
	sock = Allocate(SAL_Socket); // delay allocation until it's needed
	SAL_Socket_Initialize(sock);
	sock->RawSocket = sock_fd;
	sock->Connected = true;

	return sock;
#elif defined POSIX
	SAL_Socket *sock;
	int sock_fd;
	struct addrinfo *server, hints;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // IPvWhatever
	hints.ai_socktype = SOCK_STREAM; // TCP

	if (getaddrinfo(address, port, &hints, &server) != 0) {
		return NULL;
	}

	sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);

	if (connect(sock_fd, server->ai_addr, server->ai_addrlen) == -1) {
		freeaddrinfo(server);
		return NULL;
	}

	freeaddrinfo(server);
	sock = Allocate(SAL_Socket); // delay allocation until it's needed
	SAL_Socket_Initialize(sock);
	sock->RawSocket = sock_fd;
	sock->Connected = true;
	return sock;
#endif
}

/**
 * Create a listening socket on all interfaces.
 *
 * @param port String with the port number or name (e.g, "http" or "80")
 * @returns a socket you can call @ref SAL_Socket_Accept on
 */
SAL_Socket* SAL_Socket_Listen(const int8* const port) {
#ifdef WINDOWS
	SAL_Socket* sock;
	SOCKET sock_fd;
	struct addrinfo *server, hints;

	if (!winsockInitialized) {
		WSADATA startupData;
		WSAStartup(514, &startupData);
		winsockInitialized = true;
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // IPvWhatever
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &server) != 0) {
		return NULL;
	}

	sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sock_fd == -1) {
		goto error;
	}

	if (bind(sock_fd, server->ai_addr, (int)server->ai_addrlen) != 0) {
		goto error;
	}

	if (listen(sock_fd, SOMAXCONN) != 0) {
		goto error;
	}
	
	sock = Allocate(SAL_Socket);
	SAL_Socket_Initialize(sock);
	sock->RawSocket = sock_fd;
	sock->Connected = true;
	return sock;

error:
	closesocket(sock_fd); // It might be invalid, who cares?
	freeaddrinfo(server);
	return NULL;
#elif defined POSIX
	SAL_Socket *sock;
	int sock_fd;
	struct addrinfo *server, hints;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC; // IPvWhatever
	hints.ai_socktype = SOCK_STREAM; // TCP
	hints.ai_flags = AI_PASSIVE;

	if (getaddrinfo(NULL, port, &hints, &server) != 0) {
		return NULL;
	}

	sock_fd = socket(server->ai_family, server->ai_socktype, server->ai_protocol);
	if (sock_fd == -1) {
		goto error;
	}

	if (bind(sock_fd, server->ai_addr, server->ai_addrlen) != 0) {
		goto error;
	}

	if (listen(sock_fd, SOMAXCONN) != 0) {
		goto error;
	}
	
	sock = Allocate(SAL_Socket);
	SAL_Socket_Initialize(sock);
	sock->RawSocket = sock_fd;
	sock->Connected = true;
	return sock;

error:
	close(sock_fd); // It might be invalid, who cares?
	freeaddrinfo(server);
	return NULL;
#endif
}

/**
 * Accept an incoming connection on a listening socket (one created by @ref
 * SAL_Socket_Listen).
 *
 * @param listener The listening socket to accept a connection on
 * @param acceptedAddress 4 network bytes representing the client's IP address
 *
 * @warning This function is currently broken and platform-dependent
 */
SAL_Socket* SAL_Socket_Accept(SAL_Socket* listener, uint32* const acceptedAddress) {
#ifdef WINDOWS
	SAL_Socket* socket;
	SOCKET rawSocket;
	SOCKADDR_IN remoteAddress;
	int32 addressLength = sizeof(remoteAddress);

	rawSocket = accept((SOCKET)listener->RawSocket, (SOCKADDR*)&remoteAddress, &addressLength);
	if (rawSocket != INVALID_SOCKET) {
		*acceptedAddress = remoteAddress.sin_addr.S_un.S_addr;

		socket = Allocate(SAL_Socket);
		SAL_Socket_Initialize(socket);
		socket->RawSocket = rawSocket;
		socket->Connected = true;
		return socket;
	}

	return NULL;
#elif defined POSIX
	SAL_Socket *sock;
	int sock_fd;

	sock_fd = accept(listener->RawSocket, NULL, NULL);
	if (sock_fd == -1) {
		return NULL;
	}

	sock = Allocate(SAL_Socket);
	SAL_Socket_Initialize(sock);
	sock->RawSocket = sock_fd;
	sock->Connected = true;
	return sock;
#endif
}

/**
 * Disconnect and close the socket.
 *
 * @param socket Socket to close
 */
void SAL_Socket_Close(SAL_Socket* socket) {
	assert(socket != NULL);

	SAL_Socket_UnsetSocketCallback(socket);
	socket->Connected = false;
#ifdef WINDOWS
	shutdown((SOCKET)socket->RawSocket, SD_BOTH);
	closesocket((SOCKET)socket->RawSocket);
	socket->RawSocket = INVALID_SOCKET;
#elif defined POSIX
	shutdown(socket->RawSocket, SHUT_RDWR);
	close(socket->RawSocket);
	socket->RawSocket = -1;
#endif
}

/**
 * Read up to @a bufferSize bytes into @a buffer from @a socket.
 *
 * @param socket Socket to read from
 * @param buffer Address to write the read data too
 * @param bufferSize Size of @a buffer
 * @returns Number of bytes read
 */
uint32 SAL_Socket_Read(SAL_Socket* socket, uint8* const buffer, const uint32 bufferSize) {
	int32 received;

	assert(buffer != NULL);
	assert(socket != NULL);

#ifdef WINDOWS
	received = recv((SOCKET)socket->RawSocket, (int8* const)buffer, bufferSize, 0);
#elif defined POSIX
	received = recv(socket->RawSocket, (int8* const)buffer, bufferSize, 0);
#endif

	if (received <= 0)
		return 0;

	return (uint32)received;
}

/**
 * Send @a writeAmount bytes from @a toWrite over @a socket.
 *
 * @param socket Socket to write to
 * @param toWrite Buffer to write from
 * @param writeAmount Number of bytes to write
 * @returns true if the call was successful, false if it failed.
 */
boolean SAL_Socket_Write(SAL_Socket* socket, const uint8* const toWrite, const uint32 writeAmount) {
	int32 result;

	assert(socket != NULL);
	assert(toWrite != NULL);

#ifdef WINDOWS
	result = send((SOCKET)socket->RawSocket, (const int8*)toWrite, writeAmount, 0);
#elif defined POSIX
	result = send(socket->RawSocket, (const int8*)toWrite, writeAmount, 0);
#endif

	return result;
}

/**
 * Register @a callback to be called whenever data is available on @a socket.
 *
 * @param socket Socket to read from
 * @param callback The callback to call
 *
 * @warning The buffer passed to @a callback is the internal buffer. Do not reference it outside out the callback. 
 */
void SAL_Socket_SetReadCallback(SAL_Socket* socket, SAL_Socket_ReadCallback callback, void* const state) {
	assert(socket != NULL);
	assert(callback != NULL);
	assert(state != NULL);

	if (!asyncWorkerRunning) {
		SAL_Socket_CallbackWorker_Initialize();
		asyncWorkerRunning = true;
	}

	if (!socket->ReadCallback)
		AsyncLinkedList_Append(&asyncSocketList, socket);
	
	socket->ReadCallback = callback;
	socket->ReadCallbackState = state;
}

/**
 * Unregisters all callbacks for @a socket.
 *
 * @param socket The socket to clear all callbacks from
 */
void SAL_Socket_UnsetSocketCallback(SAL_Socket* socket) {
	assert(socket != NULL);

	if (socket->ReadCallback) {
		socket->ReadCallback = NULL;
		socket->ReadCallbackState = NULL;
		
		AsyncLinkedList_Remove(&asyncSocketList, socket);
	}
}

/**
 * Clears all the registered callbacks.
 */
void SAL_Socket_ClearCallbacks(void) {
	SAL_Socket_CallbackWorker_Shutdown();
}
