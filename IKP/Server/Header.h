#include <ws2tcpip.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#pragma warning(disable:4996) //disejbluje warning taj
#pragma comment(lib, "Ws2_32.lib") //linker switch da prorade socketi

#define DEFAULT_PORT "27016"
#define DEFAULT_BUFLEN 150

typedef struct elt {
	struct elt* next;
	char* value;
}elt;

typedef struct queue {
	struct elt* head;  /* dequeue this next */
	struct elt* tail;  /* enqueue after this */
}queue;

typedef struct queuepair
{
	int idclienta;
	char* nazivreda;
	HANDLE handle;
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

	int brojizabranihservisa;
	struct queuepair* queuepair;
	struct List* next;
} List;

char* GetQueuePairNames(List* lista);
int queueEmpty(const struct queue* q);
List* ListElementAt(int index, List* head);
void ListAdd(int number, queuepair *qpr, List** head);
int ListCount(List* head);
void ClientShutdown(List* client);
void queuePrint(struct queue* q);
struct queue* queueCreate();
struct queuepair* queuePairCreate();
struct queuepair* queuePairCreate(char *naziv);
void enq(struct queue* q, char* value);
void queueDestroy(struct queue* q);
void deq(struct queue* q);
char* deq2(struct queue* q, char* poruka);
bool InitializeWindowsSockets();
DWORD WINAPI ClientChooseQueuePair(LPVOID lpParam);
DWORD WINAPI ServerCommunicateThread(LPVOID lpParam);
void Select(SOCKET socket, bool read);