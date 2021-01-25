#include <ws2tcpip.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#pragma warning(disable:4996) //disejbluje warning taj
#pragma comment(lib, "Ws2_32.lib") //linker switch da prorade socketi

#define DEFAULT_PORT "27016"
#define DEFAULT_PORT2 27017
#define DEFAULT_PORT3 "27017"

#define DEFAULT_BUFLEN 150

typedef struct elt {
	struct elt* next;
	char* value;
}elt;

typedef struct queue {
	struct elt* head;  /* dequeue this next */
	struct elt* tail;  /* enqueue after this */
	DWORD dword;
	HANDLE h;
}queue;

typedef struct queuepair
{
	int idclienta;
	char* nazivreda;
	int shutdown;
	struct queue* queuesendtoserv;
	struct queue* queuerecvfromserv;
}queuepair;

typedef struct List
{
	int num;
	SOCKET s;
	DWORD threadID;
	HANDLE clienth;
	int ready;
	struct queuepair* queuepair;
	struct List* next;
} List;

typedef struct LhreadParams {
	SOCKET clientsocket;
	char* naziv;
}ThreadParams;

DWORD WINAPI SendQueueThread(LPVOID lpParam);
DWORD WINAPI RecvQueueThread(LPVOID lpParam);
char* GetQueuePairNames(List* lista);
int queueEmpty(const struct queue* q);
List* ListElementAt(char naziv[], List* head);
void ListAdd(char* c, SOCKET s, DWORD id, HANDLE h, List** head);
int ListCount(List* head);
void ClientShutdown(List* client);
void queuePrint(struct queue* q);
struct queue* queueCreate(int sw, char* naziv);
struct queuepair* queuePairCreate();
struct queuepair* queuePairCreate(char *naziv);
void enq(struct queue* q, char* value);
void queueDestroy(struct queue* q);
void deq(struct queue* q);
char* deq2(struct queue* q, char* poruka);
bool InitializeWindowsSockets();
DWORD WINAPI ClientChooseQueuePair(LPVOID lpParam);
DWORD WINAPI ClientServerCommunicateThread(LPVOID lpParam);
DWORD WINAPI ServerToServer1(LPVOID lpParam);
DWORD WINAPI ServerToServer2(LPVOID lpParam);
void Select(SOCKET socket, bool read);