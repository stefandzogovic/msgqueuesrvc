#include "Header.h"


extern int idClienta;
extern SOCKET connectSocket;
extern char buffer[DEFAULT_BUFLEN];


int __cdecl main(int argc, char** argv)
{
	int iResult;
	char messageToSend[] = "this is a test";
	int i = 0;

	memset(buffer, 0, DEFAULT_BUFLEN);

	if (InitializeWindowsSockets() == false)
	{
		// we won't log anything since it will be logged
		// by InitializeWindowsSockets() function
		return 1;
	}
	// create a socket
	connectSocket = socket(AF_INET,
		SOCK_STREAM,
		IPPROTO_TCP);

	if (connectSocket == INVALID_SOCKET)
	{
		printf("socket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
		return 1;
	}

	sockaddr_in serverAddress;
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_addr.s_addr = inet_addr("127.0.0.1");
	serverAddress.sin_port = htons(DEFAULT_PORT);
	// connect to server specified in serverAddress and socket connectSocket
	if (connect(connectSocket, (SOCKADDR*)&serverAddress, sizeof(serverAddress)) == SOCKET_ERROR)
	{
		printf("Unable to connect to server.\n");
		closesocket(connectSocket);
		WSACleanup();
	}

	iResult = recv(connectSocket, buffer, DEFAULT_BUFLEN, 0);
	printf(buffer);

	printf("Choose commoncuation channel: \n");
	char returnstring[1000];
	scanf("%s", returnstring);
   	//izabraniservisizaconnect = Menu(connectSocket, buffer);

	//Connect(connectSocket, izabraniservisizaconnect);

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	//HANDLE h1;
	//DWORD dw1;
	//h1 = CreateThread(NULL, 0, &ClientSendData, NULL, 0, &dw1);
	//// Send an prepared message with null terminator included
	//scanf("%d", &i);

	// cleanup
	shutdown(connectSocket, SD_RECEIVE);
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}