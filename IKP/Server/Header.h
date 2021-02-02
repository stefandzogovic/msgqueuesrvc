#include <ws2tcpip.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>

#pragma warning(disable:4996) //disejbluje warning taj
#pragma comment(lib, "Ws2_32.lib") //linker switch da prorade socketi

#define DEFAULT_PORT "27013"
#define DEFAULT_PORT2 27019
#define DEFAULT_PORT3 "27019"
#define MAX_QUEUEPAIRS 50

#define DEFAULT_BUFLEN 15000

//struktura za poruku (elt//element)
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
	int idclienta; //ne koristi se
	char* nazivreda;
	int shutdown; //msm da se ne koristi
	struct queue* queuesendtoserv;
	struct queue* queuerecvfromserv;
}queuepair;

//struktura za svaki par redova + klijent (par par redova i klijent) sadrzi RCT thread sa slike za komunikaciju sa klijentom
typedef struct List
{
	int num; // ne koristi se
	SOCKET s;
	DWORD threadID;
	HANDLE clienth;
	int ready; //da li je klijent konektovan ili su slobodni parovi redova (0 ili 1)
	struct queuepair* queuepair;
	struct List* next;
} List;

//struktura za proslednjivanje na thread (socket i naziv para  redova)
typedef struct LhreadParams {
	SOCKET clientsocket;
	char* naziv;
}ThreadParams;


char* GetFreeQueuePairNames(List* lista); //ne koristi se 
char* GetAllQueuePairNames(List* head); // funkcija za vracanje svih parova redova na serveru 
int queueEmpty(const struct queue* q); //provera da li je red prazan, koristi se kod enqueue i dequeue
List* ListElementAt(char naziv[], List* head); //vraca elemenat tipa List koji ima taj par redova
void ListAdd(char* c, SOCKET s, DWORD id, HANDLE h, List** head, int ready);
int ListCount(List* head);
void ClientShutdown(List* client); //ne koristi se nigde
void queuePrint(struct queue* q); //ne koristi se nigde
struct queue* queueCreate(int sw, char* naziv); //int sw je da li je send queue ili recv queue (0 ili 1)
//struct queuepair* queuePairCreate(); //ne koriti se
struct queuepair* queuePairCreate(char *naziv);
void ParseQueuePairNames(char* buffer); //parsira nazive parova redova koje je poslao drugi server
void enq(struct queue* q, char* value); //enqueue
void queueDestroy(struct queue* q); // ne koristim
void deq(struct queue* q); // ne koristm
char* deq2(struct queue* q, char* poruka); //dequeue
bool InitializeWindowsSockets(); //inicijalizacija socketa


DWORD WINAPI SendQueueThread(LPVOID lpParam);
DWORD WINAPI RecvQueueThread(LPVOID lpParam);
DWORD WINAPI ClientChooseQueuePair(LPVOID lpParam);
DWORD WINAPI ServerEnqueueThread(LPVOID lpParam);
DWORD WINAPI ServerDequeueThread(LPVOID lpParaml);
DWORD WINAPI ServerToServer1(LPVOID lpParam); //salje svoje redove drugom serveru (send funkcija) na svakih 5 sekundi 
DWORD WINAPI ServerToServer2(LPVOID lpParam); //recv u while petlji prihvata nazive redova sa drugog servera RECVT
void Select(SOCKET socket, bool read); 