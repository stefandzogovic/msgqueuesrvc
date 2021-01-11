//#define WIN32_LEAN_AND_MEAN

#include <ws2tcpip.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#pragma warning(disable:4996) 
#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 150
#define DEFAULT_PORT 27016

void Connect(SOCKET, char* serviceIds);
bool InitializeWindowsSockets();