#include "Header.h"

int idClienta = 0;
SOCKET connectSocket = INVALID_SOCKET;
char buffer[DEFAULT_BUFLEN];

bool InitializeWindowsSockets()
{
	WSADATA wsaData;
	// Initialize windows sockets library for this process

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("WSAStartup failed with error: %d\n", WSAGetLastError());
		return false;
	}
	return true;
}

void Connect(SOCKET connectSocket, char* izabraniservisi)
{
	//int iResult = 0;
	//iResult = send(connectSocket, izabraniservisi, brojservisa * 4, 0); //(brojservisa + 1) * 4

	//if (iResult == SOCKET_ERROR)
	//{
	//	int i = 0;
	//	printf("send failed with error: %d\n", WSAGetLastError());
	//	closesocket(connectSocket);
	//	WSACleanup();
	//	scanf("%d", &i);
	//	return;
	//}
	//Sleep(1000);
}