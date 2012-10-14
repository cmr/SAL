/**
 * @file Socket.c
 * @brief TCP networking functions
 */
#include "Socket.h"

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

static struct _CallbackState {
    SAL_Socket Socket;
    SAL_Socket_ReadCallback Callback;
    uint8 Buffer[CALLBACK_BUFFER_SIZE];
};
typedef struct _CallbackState CallbackState;

static LinkedList callbackList;
static SAL_Mutex callbackListMutex;
static SAL_Thread worker;
static boolean workerInitialized = false;
static boolean workerRunning = false;

static SAL_Thread_Start(Worker_Run) {
    fd_set readSet;
    uint32 i;
    uint32 bytesRead;
    CallbackState* currentState;
    LinkedList_Iterator* selectIterator;
    LinkedList_Iterator* readyIterator;
    
    selectIterator = LinkedList_BeginIterate(&callbackList);
    readyIterator = LinkedList_BeginIterate(&callbackList);

    while (workerRunning) {
        FD_ZERO(&readSet);

        SAL_Mutex_Acquire(callbackListMutex);

        for (i = 0; i < FD_SETSIZE; i++)
            if ( LinkedList_IterateNext(currentState, selectIterator, CallbackState) ) {
                FD_SET(currentState->Socket, &readSet);
            }
            else {
                LinkedList_ResetIterator(selectIterator);            
                break;
            }

        select(0, &readSet, NULL, NULL, NULL);

        for (i = 0; i < readSet.fd_count; i++) {
            LinkedList_ForEach(currentState, readyIterator, CallbackState)
                if (currentState->Socket == readSet.fd_array[i]) {
                    bytesRead = SAL_Socket_Read(currentState->Socket, currentState->Buffer, CALLBACK_BUFFER_SIZE);
                    currentState->Callback(currentState->Buffer, bytesRead);
                }
        }

        SAL_Mutex_Release(callbackListMutex);

        SAL_Thread_Sleep(25);
    }
    
    LinkedList_EndIterate(selectIterator);
    LinkedList_EndIterate(readyIterator);

    return 0;
}

static void Worker_Initialize() {
    LinkedList_Initialize(&callbackList, NULL);
    callbackListMutex = SAL_Mutex_Create();
    workerRunning = true;
    worker = SAL_Thread_Create(Worker_Run, NULL);
}



/**
 * Create a TCP connection to a host.
 *
 * @param address A string specifying the IP address to connect to
 * @param port Port to connect to
 */
SAL_Socket SAL_Socket_Connect(const int8* address, uint16 port) {
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
        serverAddress.sin_addr.S_un.S_addr = inet_addr(address);
        serverAddress.sin_port = htons(port);

        if (connect(server, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
            return NULL;

        return server;
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
	
	    return listener;
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
	    SOCKET acceptedSocket;
	    SOCKADDR_IN remoteAddress;
	    int32 addressLength = sizeof(remoteAddress);
        
		acceptedSocket = accept(listener, (SOCKADDR*)&remoteAddress, &addressLength);
		if (acceptedSocket != INVALID_SOCKET) {
            *acceptedAddress = remoteAddress.sin_addr.S_un.S_addr;
            return acceptedSocket;
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
        shutdown(socket, SD_BOTH);
	    closesocket(socket);
    #elif defined POSIX

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
uint32 SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint32 bufferSize) {
    #ifdef WINDOWS
        assert(buffer);

        return (uint32)recv(socket, (int8*)buffer, bufferSize, 0);
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
        ioctlsocket(socket, FIONBIO, &mode);

        result = send(socket, (const int8*)toWrite, writeAmount, 0);
        
        mode = 0;
        ioctlsocket(socket, FIONBIO, &mode);

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
void SAL_Socket_RegisterReadCallback(SAL_Socket socket, SAL_Socket_ReadCallback callback) {
    CallbackState* callbackData;
    
    assert(socket);
    assert(callback);

    if (!workerInitialized) {
        Worker_Initialize();
        workerInitialized = true;
    }

    callbackData = Allocate(CallbackState);
    callbackData->Socket = socket;
    callbackData->Callback = callback;

    SAL_Mutex_Acquire(callbackListMutex);
    LinkedList_Append(&callbackList, callbackData);
    SAL_Mutex_Release(callbackListMutex);
}
