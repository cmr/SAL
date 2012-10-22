/** vim: set noet ci sts=0 sw=4 ts=4
 * @file Socket.c
 * @brief TCP networking functions
 */
#include "Socket.h"

#include <Utilities/Lookup.h>
#include <Utilities/LinkedList.h>
#include <Utilities/Memory.h>
#include "Thread.h"

#define CALLBACK_BUFFER_SIZE 1024

#ifdef WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define FD_SETSIZE 1024
	#include <Windows.h>
	#include <winsock2.h>
	#include <ws2tcpip.h>

	static boolean winsockInitialized = false;
#endif

static struct AsyncSocketEntry {
	SAL_Socket Socket;
	LinkedList Callbacks;
};
typedef struct AsyncSocketEntry AsyncSocketEntry;

static struct CallbackEntry {
	SAL_Socket_ReadCallback Callback;
	void* State;
};
typedef struct CallbackEntry CallbackEntry;

static uint8 asyncSocketBuffer[CALLBACK_BUFFER_SIZE];
static LinkedList asyncSocketList;
static Lookup asyncSocketLookup;
static SAL_Mutex asyncSocketMutex;
static SAL_Thread asyncWorker;
static boolean asyncWorkerRunning = false;

static SAL_Thread_Start(AsyncWorker_Run) {
	fd_set readSet;
	uint32 i;
	uint32 bytesRead;
	AsyncSocketEntry* socketEntry;
	CallbackEntry* callbackEntry;
	SAL_Socket socket;
	LinkedList_Iterator* selectIterator;

	selectIterator = LinkedList_BeginIterate(&asyncSocketList);

	while (asyncWorkerRunning) {
		FD_ZERO(&readSet);

		SAL_Mutex_Acquire(asyncSocketMutex);

		/* iterates over all sockets with registered callbacks. It either finishes when 1024 sockets have been added or the socket list is exhausted. If the socket list is greater than 1024, the position is remembered on the next loop   */
		for (i = 0; i < FD_SETSIZE && LinkedList_IterateNext(socket, selectIterator, SAL_Socket); i++) {
			#ifdef WINDOWS
				FD_SET((SOCKET)socket, &readSet);
			#else defined POSIX

			#endif
		}
		LinkedList_ResetIterator(selectIterator);

		select(0, &readSet, NULL, NULL, NULL);

		for (i = 0; i < readSet.fd_count; i++) {
			socketEntry = Lookup_Find(&asyncSocketLookup, readSet.fd_array[i], AsyncSocketEntry*); /* find the entry owned by the ready socket */
			if (socketEntry) {
				bytesRead = SAL_Socket_Read(socketEntry->Socket, asyncSocketBuffer, CALLBACK_BUFFER_SIZE);
				LinkedList_ForEach(callbackEntry, &socketEntry->Callbacks, CallbackEntry*) { /* for every callback associated with the socket, invoke the callback */
					callbackEntry->Callback(asyncSocketBuffer, bytesRead, callbackEntry->State);
				}
			}
		}

		SAL_Mutex_Release(asyncSocketMutex);
		SAL_Thread_Sleep(25);
	}

	LinkedList_EndIterate(selectIterator);

	return 0;
}

static void AsyncWorker_Initialize() {
	Lookup_Initialize(&asyncSocketLookup, Memory_Free);
	LinkedList_Initialize(&asyncSocketList, NULL);
	asyncSocketMutex = SAL_Mutex_Create();
	asyncWorkerRunning = true;
	asyncWorker = SAL_Thread_Create(AsyncWorker_Run, NULL);
}

static void AsyncWorker_Shutdown() {
	asyncWorkerRunning = false;
	SAL_Thread_Join(asyncWorker);
	SAL_Mutex_Free(asyncSocketMutex);
	Lookup_Uninitialize(&asyncSocketLookup);
	LinkedList_Uninitialize(&asyncSocketList);
}


/**
 * Create a TCP connection to a host.
 *
 * @param address A string specifying the hostname to connect to
 * @param port Port to connect to
 */
SAL_Socket SAL_Socket_Connect(const int8* address, uint16 port) {
#ifdef WINDOWS
	unsigned long hostAddress;
	HOSTENT* hostEntry;
	uint8 i;
	SAL_Socket socket;

	if (!winsockInitialized) {
		WSADATA startupData;
		WSAStartup(514, &startupData);
		winsockInitialized = true;
	}

	hostAddress = inet_addr(address);
	if (hostAddress == INADDR_NONE) {
		hostEntry = gethostbyname(address);
		for (i = 0, socket = NULL; i < hostEntry->h_length; i++)
			if (socket = SAL_Socket_ConnectIP((uint32)hostEntry->h_addr_list[i], port))
				return socket;

		return NULL;
	}
	else {
		return SAL_Socket_ConnectIP(hostAddress, port);
	}
#elif defined POSIX

#endif
}

/**
 * Create a TCP connection to a host.
 *
 * @param ip IP Address of the remote host
 * @param port Port to connect to
 */
SAL_Socket SAL_Socket_ConnectIP(uint32 ip, uint16 port) {
#ifdef WINDOWS
	SOCKET server;
	SOCKADDR_IN serverAddress;

	if (!winsockInitialized) {
		WSADATA startupData;
		WSAStartup(514, &startupData);
		winsockInitialized = true;
	}

	server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server == INVALID_SOCKET)
		return NULL;

	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.S_un.S_addr = (unsigned long)ip;
	serverAddress.sin_port = htons(port);

	if (connect(server, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
		return NULL;

	return (SAL_Socket)server;
#elif defined POSIX

#endif
}

/**
 * Create a listening socket on all interfaces.
 *
 * @param port String with the port number or name (e.g, "http" or "80")
 * @returns a socket you can call @ref SAL_Socket_Accept on
 */
SAL_Socket SAL_Socket_Listen(const int8* port) {
#ifdef WINDOWS
	ADDRINFO* addressInfo;
	ADDRINFO hints;
	SOCKET listener;

	if (!winsockInitialized) {
		WSADATA startupData;
		WSAStartup(514, &startupData);
		winsockInitialized = true;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	if (!getaddrinfo(NULL, port, &hints, &addressInfo)) {
		freeaddrinfo(addressInfo);
		return NULL;
	}

	listener = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
	if (listener == INVALID_SOCKET) {
		freeaddrinfo(addressInfo);
		return NULL;
	}

	if (!bind(listener, addressInfo->ai_addr, (int32)addressInfo->ai_addrlen)) {
		freeaddrinfo(addressInfo);
		closesocket(listener);
		return NULL;
	}

	freeaddrinfo(addressInfo);

	if (!listen(listener, SOMAXCONN))
		return NULL;

	return (SAL_Socket)listener;
#elif defined POSIX

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
SAL_Socket SAL_Socket_Accept(SAL_Socket listener, uint32* acceptedAddress) {
#ifdef WINDOWS
	SOCKET rawSocket;
	SOCKADDR_IN remoteAddress;
	int32 addressLength = sizeof(remoteAddress);
	
	rawSocket = accept((SOCKET)listener, (SOCKADDR*)&remoteAddress, &addressLength);
	if (rawSocket != INVALID_SOCKET) {
		*acceptedAddress = remoteAddress.sin_addr.S_un.S_addr;
		return (SAL_Socket)rawSocket;
	}

	return NULL;
#elif defined POSIX

#endif
}

/**
 * Disconnect and close the socket.
 *
 * @param socket Socket to close
 */
void SAL_Socket_Close(SAL_Socket socket) {
#ifdef WINDOWS
	shutdown((SOCKET)socket, SD_BOTH);
	closesocket((SOCKET)socket);
#elif defined POSIX

#endif

	SAL_Mutex_Acquire(asyncSocketMutex);

	Lookup_Remove(&asyncSocketLookup, (uint64)socket);
	LinkedList_Remove(&asyncSocketList, socket);

	if (asyncSocketList.Count == 0) {
		SAL_Mutex_Release(asyncSocketMutex);
		AsyncWorker_Shutdown();
	}
	else {
		SAL_Mutex_Release(asyncSocketMutex);
	}
}

/**
 * Read up to @a bufferSize bytes into @a buffer from @a socket.
 *
 * @param socket Socket to read from
 * @param buffer Address to write the read data too
 * @param bufferSize Size of @a buffer
 * @returns Number of bytes read
 */
uint32 SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint32 bufferSize) {
#ifdef WINDOWS
	int32 received;

	assert(buffer);

	received = recv((SOCKET)socket, (int8*)buffer, bufferSize, 0);
	if (received <= 0)
		return 0;

	return (uint32)received;
#elif defined POSIX

#endif
}

/**
 * Send @a writeAmount bytes from @a toWrite over @a socket.
 *
 * @param socket Socket to write to
 * @param toWrite Buffer to write from
 * @param writeAmount Number of bytes to write
 * @returns true if the call was successful, false if it failed.
 */
boolean SAL_Socket_Write(SAL_Socket socket, const uint8* toWrite, uint32 writeAmount) {
#ifdef WINDOWS
	unsigned long mode;
	int32 result;

	assert(toWrite);

	mode = 1;
	ioctlsocket((SOCKET)socket, FIONBIO, &mode);

	result = send((SOCKET)socket, (const int8*)toWrite, writeAmount, 0);

	mode = 0;
	ioctlsocket((SOCKET)socket, FIONBIO, &mode);

	return result != SOCKET_ERROR;
#elif defined POSIX

#endif
}

/**
 * Register @a callback to be called whenever data is available on @a socket.
 *
 * @param socket Socket to read from
 * @param callback The callback to call
 *
 * @warning The buffer passed to @a callback is the internal buffer. Do not reference it outside out the callback. 
 */
void SAL_Socket_RegisterReadCallback(SAL_Socket socket, SAL_Socket_ReadCallback callback, void* state) {
	AsyncSocketEntry* socketEntry;
	CallbackEntry* callbackEntry;

	assert(socket);
	assert(callback);

	if (!asyncWorkerRunning) {
		AsyncWorker_Initialize();
		asyncWorkerRunning = true;
	}

	callbackEntry = Allocate(CallbackEntry);
	callbackEntry->Callback = callback;
	callbackEntry->State = state;

	SAL_Mutex_Acquire(asyncSocketMutex);

	if (socketEntry = Lookup_Find(&asyncSocketLookup, (uint64)socket, AsyncSocketEntry*)) {
		LinkedList_Append(&socketEntry->Callbacks, callbackEntry);
	}
	else {
		socketEntry = Allocate(AsyncSocketEntry);
		socketEntry->Socket = socket;
		LinkedList_Initialize(&socketEntry->Callbacks, Memory_Free);
		LinkedList_Append(&socketEntry->Callbacks, callbackEntry);
		Lookup_Add(&asyncSocketLookup, (uint64)socket, socketEntry, true);
	}

	if (!LinkedList_Find(&asyncSocketList, socket, SAL_Socket))
		LinkedList_Append(&asyncSocketList, socket);

	SAL_Mutex_Release(asyncSocketMutex);
}

/**
 * Unregisters all callbacks for @a socket.
 *
 * @param socket The socket to clear all callbacks from
 */
void SAL_Socket_UnregisterSocketCallbacks(SAL_Socket socket) {
	SAL_Mutex_Acquire(&asyncSocketMutex);

	LinkedList_Remove(&asyncSocketList, socket);
	Lookup_Remove(&asyncSocketLookup, (uint64)socket);

	SAL_Mutex_Release(&asyncSocketMutex);
}
