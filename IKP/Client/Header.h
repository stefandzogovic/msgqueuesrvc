#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS

#include <ws2tcpip.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#pragma warning(disable:4996) 
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 150
#define DEFAULT_PORT 27016

//typedef struct LhreadParams {
//	SOCKET clientsocket;
//}ThreadParams;

void Connect(SOCKET, char* serviceIds);
DWORD WINAPI ClientSendData(LPVOID lpParam);
DWORD WINAPI ClientRecvData(LPVOID lpParam);
bool InitializeWindowsSockets();