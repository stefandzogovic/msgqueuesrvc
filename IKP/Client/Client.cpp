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
	char delim[] = ",";
	char* ptr = strtok(buffer, delim);
	int brojac = 1;
	while (ptr != NULL)
	{
		printf("%d. %s\n", brojac, ptr);
		ptr = strtok(NULL, delim);
		brojac++;
	}

	printf("Choose commoncuation channel: \n");
	char string[1000] = "";
	(void)fgets(string, 999, stdin);

	if ((strlen(string) > 0) && (string[strlen(string) - 1] == '\n'))
		string[strlen(string) - 1] = '\0';

	iResult = send(connectSocket, string, strlen(string), 0);


	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(connectSocket, FIONBIO, &nonBlockingMode);

	HANDLE h1, h2;
	DWORD dw1, dw2;
	h1 = CreateThread(NULL, 0, &ClientSendData, NULL, 0, &dw1);
	h2 = CreateThread(NULL, 0, &ClientRecvData, NULL, 0, &dw2);

	while (true)
	{

	}
	shutdown(connectSocket, SD_RECEIVE);
	closesocket(connectSocket);
	WSACleanup();

	return 0;
}