#include "Header.h"


bool InitializeWindowsSockets();

int main()
{
	// Socket used for listening for new clients 
	SOCKET listenSocket = INVALID_SOCKET;
	// Socket used for communication with client
	SOCKET acceptedSocket = INVALID_SOCKET;

	SOCKET connectSocket = INVALID_SOCKET;

	extern SOCKET serverSocket;

	extern char* strings[MAX_QUEUEPAIRS];
	for (int i = 0; i < MAX_QUEUEPAIRS; i++)
	{
		strings[i] = NULL;
	}
	extern CRITICAL_SECTION cs;
	extern List* lista;
	int iResult;

	int id = 0;

	InitializeCriticalSection(&cs);

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}

	int x;
	printf("Unesi broj parova redova: ");
	scanf("%d", &x);
	for (int i = 0; i < x; i++)
	{
		char* c = (char*)malloc(50);
		printf("\nIme reda: ");
		scanf("%s", c);
		ListAdd(c, NULL, NULL, NULL, &lista, 0);
	}
	printf("Server initialized.\n\n");

	printf("1. Listen for second server.\n");
	printf("2. Connect to second server.\n");
	int br = 0;
	(void)scanf("%d", &x);
#pragma region

	if (x == 1)
	{
		connectSocket = socket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP);

		if (connectSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		// Prepare address information structures
		addrinfo* resultingAddress = NULL;
		addrinfo hints;

		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_INET;       // IPv4 address
		hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
		hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
		hints.ai_flags = AI_PASSIVE;     // 

		// Resolve the server address and port
		iResult = getaddrinfo(NULL, DEFAULT_PORT3, &hints, &resultingAddress);
		if (iResult != 0)
		{
			printf("getaddrinfo failed with error: %d\n", iResult);
			WSACleanup();
			return 1;
		}

		// Create a SOCKET for connecting to server
		listenSocket = socket(AF_INET,      // IPv4 address famly
			SOCK_STREAM,  // stream socket
			IPPROTO_TCP); // TCP

		if (listenSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			WSACleanup();
			return 1;
		}

		// Setup the TCP listening socket - bind port number and local address 
		// to socket
		iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
		if (iResult == SOCKET_ERROR)
		{
			printf("bind failed with error: %d\n", WSAGetLastError());
			freeaddrinfo(resultingAddress);
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		// Since we don't need resultingAddress any more, free it
		freeaddrinfo(resultingAddress);

		iResult = listen(listenSocket, SOMAXCONN);
		if (iResult == SOCKET_ERROR)
		{
			printf("listen failed with error: %d\n", WSAGetLastError());
			closesocket(listenSocket);
			WSACleanup();
			return 1;
		}

		unsigned long int nonBlockingMode = 1;
		iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);

		if (iResult == SOCKET_ERROR)
		{
			printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
			return 1;
		}

#pragma endregion

		while (br == 0)
		{
			Select(listenSocket, true);
			serverSocket = accept(listenSocket, NULL, NULL);
			if (serverSocket != INVALID_SOCKET)
			{
				HANDLE h, h1;
				DWORD dw, dw1;
				h = CreateThread(NULL, 0, &ServerToServer1, (LPVOID)NULL, 0, &dw);
				h1 = CreateThread(NULL, 0, &ServerToServer2, (LPVOID)NULL, 0, &dw1);
				br++;
			}
		}
	}
	else
	{
		if (InitializeWindowsSockets() == false)
		{
			// we won't log anything since it will be logged
			// by InitializeWindowsSockets() function
			return 1;
		}
		// create a socket
		serverSocket = socket(AF_INET,
			SOCK_STREAM,
			IPPROTO_TCP);

		if (serverSocket == INVALID_SOCKET)
		{
			printf("socket failed with error: %ld\n", WSAGetLastError());
			WSACleanup();
			return 1;
		}

		sockaddr_in serverAddress;
		serverAddress.sin_family = AF_INET;
		serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
		serverAddress.sin_port = htons(DEFAULT_PORT2);
		// connect to server specified in serverAddress and socket connectSocket
		if (connect(serverSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
		{
			printf("Unable to connect to server.\n");
			closesocket(serverSocket);
			WSACleanup();
		}
		else
		{
			HANDLE h, h1;
			DWORD dw, dw1;
			h = CreateThread(NULL, 0, &ServerToServer1, (LPVOID)NULL, 0, &dw);
			h1 = CreateThread(NULL, 0, &ServerToServer2, (LPVOID)NULL, 0, &dw1);
		}
	}
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	// Prepare address information structures
	addrinfo* resultingAddress = NULL;
	addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;       // IPv4 address
	hints.ai_socktype = SOCK_STREAM; // Provide reliable data streaming
	hints.ai_protocol = IPPROTO_TCP; // Use TCP protocol
	hints.ai_flags = AI_PASSIVE;     // 

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &resultingAddress);
	if (iResult != 0)
	{
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for connecting to server
	listenSocket = socket(AF_INET,      // IPv4 address famly
		SOCK_STREAM,  // stream socket
		IPPROTO_TCP); // TCP

	if (listenSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket - bind port number and local address 
	// to socket
	iResult = bind(listenSocket, resultingAddress->ai_addr, (int)resultingAddress->ai_addrlen);
	if (iResult == SOCKET_ERROR)
	{
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(resultingAddress);
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Since we don't need resultingAddress any more, free it
	freeaddrinfo(resultingAddress);

	iResult = listen(listenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR)
	{
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(listenSocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}
	do {
		Select(listenSocket, true);

		acceptedSocket = accept(listenSocket, NULL, NULL);

		if (acceptedSocket == INVALID_SOCKET)
		{
			continue;
		}
		else {
			printf("Client Connected");
			HANDLE hThread2;
			DWORD dw2;
			int* param = (int*)malloc(sizeof(int));
			Select(acceptedSocket, false);
			char* queuepairnames = GetFreeQueuePairNames(lista);

			if (queuepairnames != NULL)
				iResult = send(acceptedSocket, queuepairnames, strlen(queuepairnames), 0);
			else
				iResult = send(acceptedSocket, "", 1, 0);

			ThreadParams* p = (ThreadParams*)malloc(sizeof(ThreadParams));
			p->clientsocket = acceptedSocket;
			hThread2 = CreateThread(NULL, 0, &ClientChooseQueuePair, (LPVOID)p, 0, &dw2);
		}
	} while (true);

	printf("this process = %d\n", GetCurrentProcessId());
	DWORD count = GetConsoleProcessList(nullptr, 0);
	while (true)
	{

	}
	return 0;
}