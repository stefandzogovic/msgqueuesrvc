#include "Header.h"

CRITICAL_SECTION cs; //Globalna promenljiva, kriticna sekcija
List* lista; //lista za paroveredova

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

struct queue* queueCreate()
{

	struct queue* q;

	q = (queue*)malloc(sizeof(struct queue));

	q->head = q->tail = 0;

	return q;
}
/*
	Funkcija za kreiranje parova redova, za odgovarajuci servis. Inicijalno su sva polja postavljena na nule, ili
	NULL (respektivno).
*/
struct queuepair* queuePairCreate()
{

	struct queuepair* q;

	q = (queuepair*)malloc(sizeof(struct queuepair));

	q->nazivreda = NULL;
	q->idclienta = 0;
	q->shutdown = 0;
	q->queuerecvfromserv = NULL;
	q->queuesendtoserv = NULL;
	q->handle = 0;
	return q;
}

/* add a new value to back of queue */
void enq(struct queue* q, char* value)
{
	struct elt* e;

	e = (elt*)malloc(sizeof(struct elt));
	assert(e);

	e->value = value;

	/* Because I will be the tail, nobody is behind me */
	e->next = 0;

	if (q->head == 0) {
		/* If the queue was empty, I become the head */
		q->head = e;
	}
	else {
		/* Otherwise I get in line after the old tail */
		q->tail->next = e;
	}

	/* I become the new tail */
	q->tail = e;
}

//jednostavna provera da li je red prazan
int queueEmpty(const struct queue* q)
{
	return (q->head == 0 || q->head == NULL);
}

/* remove and return value from front of queue */
void deq(struct queue* q)
{
	struct elt* e;

	assert(!queueEmpty(q));

	/* patch out first element */
	e = q->head;
	q->head = q->head->next;

	free(e);

}
//isto kao deq samo sto se vraca pokazivac na vrednost dequeovanog elementa
char* deq2(struct queue* q, char* poruka)
{
	struct elt* e;

	assert(!queueEmpty(q));

	memcpy(poruka, q->head->value, DEFAULT_BUFLEN);
	//poruka = q->head->value;


	/* patch out first element */
	e = q->head;
	q->head = e->next;
	//char *temp_por = poruka;
	//temp_por = temp_por + (NUMBER_OF_SERVICES + 2) * 4;

	free(e);

	return poruka;
}

/* print contents of queue on a single line, head first */
void queuePrint(struct queue* q)
{
	struct elt* e;
	int brojac = 0;
	for (e = q->head; e != 0; e = e->next) {
		brojac++;
	}
	printf("%d", brojac);
	putchar('\n');
}

/* free a queue and all of its elements */
void queueDestroy(struct queue* q)
{
	while (!queueEmpty(q)) {
		deq(q);
	}

	free(q);
	q = NULL;
}

//unistava par redova i oslobadja memoriju
void queuepairDestroy(struct queuepair* q)
{
	free(q);
	q = NULL;
}

/*
	Funkcija kao parametre prima ID servera (ili klijenta), soket za komunikaciju, i pokazivac na listu podignutih servera, ili klijenata.
	Memorija se zauzima dinamicki za svaki novi elemenat liste upotrebom funkcije malloc,
	i redom se popunjavaju polja novog elementa liste, kao i povezivanje novog elementa sa starim (postavljanje na kraj),
	ili njegovo postavljanje na pocetak liste.
	Soket za komunikaciju se koristi da bi proksi znao kojem klijentu/serveru prosledjuje poruke.
*/
void ListAdd(int number, SOCKET s, DWORD id, HANDLE h, List** head)
{
	List* el;
	el = (List*)malloc(sizeof(List));
	el->num = number;
	el->s = s;
	el->threadID = id;
	el->clienth = h;
	el->next = NULL;
	el->brojizabranihservisa = 0;
	if (*head == NULL) {
		*head = el;
	}
	else {
		List* temp = *head;
		while (temp->next != NULL) {
			temp = temp->next;
		}
		temp->next = el;
	}
}
/*
	Funkcija vraca koliko postoji elemanata u odgovarajucoj listi (parametar pokazuje koja je lista u pitanju).
*/
int ListCount(List* head)
{
	List* temp = head;
	int ret = 0;
	while (temp) {
		ret++;
		temp = temp->next;
	}
	return ret;
}

char* GetQueuePairNames(List* head)
{
	List* temp = head;
	char* buffer = (char*)malloc(sizeof(temp->queuepair->nazivreda));
	buffer = temp->queuepair->nazivreda;
	int ret = 0;
	while (temp) {
		ret++;
		temp = temp->next;
		char *_New_Buf = (char*)realloc(buffer, sizeof(temp->queuepair->nazivreda));
		char x = ',';
		if (_New_Buf != NULL)
			buffer = _New_Buf;
		else
		{

		}
	}
	return buffer;
}

/*
	Funkcija prima 2 parametra: ID klijenta ili servisa sa kojim se komunicira, kao i pokazivac na odgovarajucu listu
	(listu podignutih klijenata, ili servisa) kroz koju se iterira u cilju popunjavanja odgovarajucih polja liste.
	Funkcija se koristi za pristupanje elementima liste. U slucaju da elemenat nije pronadjen, vraca NULL; ukoliko jeste
	povratna vrednost je pokazivac na trazeni element.
*/
List* ListElementAt(int index, List* head)
{
	if (ListCount(head) > 0) {
		List* temp = head;

		while (temp->num != index) {
			temp = temp->next;
		}
		return temp;
	}
	return NULL;
}
void Select(SOCKET socket, bool read) {
	while (true) {
		// Initialize select parameters
		FD_SET set;
		timeval timeVal;
		FD_ZERO(&set);
		// Add socket we will wait to read from
		FD_SET(socket, &set);
		// Set timeouts to zero since we want select to return
		// instantaneously
		timeVal.tv_sec = 0;
		timeVal.tv_usec = 0;
		int iResult;

		if (read) {
			iResult = select(0 /* ignored */, &set, NULL, NULL, &timeVal);
		}
		else {
			iResult = select(0 /* ignored */, NULL, &set, NULL, &timeVal);
		}

		if (iResult < 0) {
			printf("Select failed");
		}
		else if (iResult == 0) {
			Sleep(100);
			continue;
		}

		return;
	}
}

DWORD WINAPI ServerCommunicateThread(LPVOID lpParam)
{
	int iResult = 0;
	int idserv = *((int*)lpParam);
	free(lpParam);
	char* buffer = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	int* pok = (int*)buffer;
	//List* servis = ListElementAt(idserv, listservicehead);

	//while (true)
	//{
	//	if (servis->ugasi == 0)
	//	{
	//		if (servis->ready == 1)
	//		{
	//			SOCKET socket = servis->s;
	//			Select(socket, true);

	//			iResult = recv(socket, buffer, DEFAULT_BUFLEN, 0);
	//			if (iResult > 0)
	//			{
	//				int idklijenta = (int)(ntohl(*pok));

	//				List* client = ListElementAt(idklijenta, listclienthead);
	//				if (client->ready == 1)
	//				{
	//					for (int j = 0; j < client->brojizabranihservisa; j++)
	//					{
	//						if (client->drajver->pokazivaci[j]->idservisa == servis->num)
	//						{
	//							EnterCriticalSection(&cs);
	//							enq(client->drajver->pokazivaci[j]->queuerecvfromserv, buffer);
	//							LeaveCriticalSection(&cs);
	//							break;
	//						}
	//					}
	//				}

	//			}
	//		}
	//	}
	//	else
	//		break;
	//}

	free(buffer);
	return 0;
}
