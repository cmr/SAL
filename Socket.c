#include "Socket.h"

#include <Utilities/LinkedList.h>
#include <Utilities/Memory.h>
#include "Thread.h"

#ifdef WINDOWS
    #define WIN32_LEAN_AND_MEAN
    #define FD_SETSIZE 1024
    #include <Windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>

    static boolean initialized = false;
#endif

typedef struct {
    SAL_Socket Socket;
    SAL_Socket_ReadCallback Callback;
    uint32 BufferSize;
    uint8* Buffer;
} AsyncWorker_Callback;

static LinkedList Callbacks;
static SAL_Mutex CallbacksMutex;
static SAL_Thread AsyncWorker;
static boolean AsyncWorkerInitialized = false;

static SAL_Thread_Start(AsyncWorker_Run) {
    fd_set readSet;
    uint32 i;
    uint32 bytesRead;
    AsyncWorker_Callback* callback;

    /*perhaps write a wrapper over FD_SET to allow for selecting over all sockets?*/
    while (true) {
        FD_ZERO(&readSet);

        LinkedList_ForEach(callback, Callbacks, AsyncWorker_Callback)
            FD_SET(callback->Socket, &readSet);

        select(0, &readSet, NULL, NULL, NULL);

        for (i = 0; i < readSet.fd_count; i++) {
            LinkedList_ForEach(callback, Callbacks, AsyncWorker_Callback)
                if (callback->Socket == readSet.fd_array[i]) {
                    bytesRead = SAL_Socket_Read(callback->Socket, callback->Buffer, callback->BufferSize);
                    callback->Callback(bytesRead);
                }
        }

        SAL_Thread_Sleep(AsyncWorker, 25);
    }
}

static void AsyncWorker_Initialize() {
    LinkedList_Initialize(&Callbacks, Memory_Free);
    
    CallbacksMutex = SAL_Mutex_Create();
    AsyncWorker = SAL_Thread_Create(AsyncWorker_Run, NULL);
}




SAL_Socket SAL_Socket_Connect(const int8* address, uint16 port) {
    #ifdef WINDOWS
        SOCKET server;
        SOCKADDR_IN serverAddress;

        if (!initialized) {
	        WSADATA startupData;
	        WSAStartup(514, &startupData);
            initialized = true;
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

SAL_Socket SAL_Socket_Listen(const int8* port) {
    #ifdef WINDOWS
	    ADDRINFO* addressInfo;
	    ADDRINFO hints;
	    SOCKET listener;

        if (!initialized) {
	        WSADATA startupData;
	        WSAStartup(514, &startupData);
            initialized = true;
        }
	    
	    ZeroMemory(&hints, sizeof(hints));
	    hints.ai_family = AF_INET;
	    hints.ai_socktype = SOCK_STREAM;
	    hints.ai_protocol = IPPROTO_TCP;
	    hints.ai_flags = AI_PASSIVE;

	    getaddrinfo(NULL, port, &hints, &addressInfo);
	    listener = socket(addressInfo->ai_family, addressInfo->ai_socktype, addressInfo->ai_protocol);
	    bind(listener, addressInfo->ai_addr, (int32)addressInfo->ai_addrlen);
        freeaddrinfo(addressInfo);
	    listen(listener, SOMAXCONN);
	
	    return listener;
    #elif defined POSIX

    #endif
}

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

void SAL_Socket_Close(SAL_Socket socket) {
    #ifdef WINDOWS
        shutdown(socket, SD_BOTH);
	    closesocket(socket);
    #elif defined POSIX

    #endif
}

uint32 SAL_Socket_Read(SAL_Socket socket, uint8* buffer, uint32 bufferSize) {
    #ifdef WINDOWS
        assert(buffer);

        return (uint32)recv(socket, (int8*)buffer, bufferSize, 0);
    #elif defined POSIX

    #endif
}

void SAL_Socket_ReadAsync(SAL_Socket socket, uint8* buffer, uint32 bufferSize, SAL_Socket_ReadCallback callback) {
    AsyncWorker_Callback* callbackData;

    if (!AsyncWorkerInitialized) {
        AsyncWorker_Initialize();
        AsyncWorkerInitialized = true;
    }

    callbackData = Allocate(AsyncWorker_Callback);
    callbackData->Socket = socket;
    callbackData->Buffer = buffer;
    callbackData->BufferSize = bufferSize;
    callbackData->Callback = callback;

    SAL_Mutex_Acquire(CallbacksMutex);
    LinkedList_Append(&Callbacks, callbackData);
    SAL_Mutex_Free(CallbacksMutex);
}

void SAL_Socket_Write(SAL_Socket socket, const uint8* toWrite, uint32 writeAmount) {
    #ifdef WINDOWS
        assert(toWrite);

        send(socket, (const int8*)toWrite, writeAmount, 0);
    #elif defined POSIX

    #endif
}
