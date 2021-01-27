#include "Header.h"

CRITICAL_SECTION cs; //Globalna promenljiva, kriticna sekcija
List* lista = NULL; //lista za paroveredova
SOCKET serverSocket = INVALID_SOCKET;
char* strings[] = { NULL };

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

DWORD WINAPI SendQueueThread(LPVOID lpParam)
{
	char* c = (char*)lpParam;
	List* el = ListElementAt(c, lista);
	char* poruka = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	while (true)
	{
		if (el != NULL)
		{
			if (!queueEmpty(el->queuepair->queuesendtoserv))
			{
				EnterCriticalSection(&cs);
				poruka = deq2(el->queuepair->queuesendtoserv, poruka);
				LeaveCriticalSection(&cs);
				//printf("poruka: %s", poruka);
			}
		}
		Sleep(15);
	}
	return NULL;
}

DWORD WINAPI RecvQueueThread(LPVOID lpParam)
{
	char* c = (char*)lpParam;
	List* el = ListElementAt(c, lista);
	char* poruka = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	int iResult;
	while (true)
	{
		if (el != NULL)
		{
			if (!queueEmpty(el->queuepair->queuerecvfromserv))
			{
				EnterCriticalSection(&cs);
				poruka = deq2(el->queuepair->queuerecvfromserv, poruka);
				LeaveCriticalSection(&cs);
				iResult = send(el->s, poruka, strlen(poruka), 0);
			}
		}
		Sleep(15);
	}
	return NULL;
}

struct queue* queueCreate(int sw, char* naziv)
{

	struct queue* q;

	q = (queue*)malloc(sizeof(struct queue));

	q->head = q->tail = 0;

	if (sw == 0)
	{
		q->h = CreateThread(NULL, 0, &SendQueueThread, (LPVOID)naziv, 0, &q->dword);
	}
	else
	{
		q->h = CreateThread(NULL, 0, &RecvQueueThread, (LPVOID)naziv, 0, &q->dword);
	}
	return q;
}
/*
	Funkcija za kreiranje parova redova, za odgovarajuci servis. Inicijalno su sva polja postavljena na nule, ili
	NULL (respektivno).
*/

struct queuepair* queuePairCreate(char* naziv)
{

	struct queuepair* q;

	q = (queuepair*)malloc(sizeof(struct queuepair));

	q->nazivreda = naziv;
	q->idclienta = 0;
	q->shutdown = 0;
	q->queuesendtoserv = queueCreate(0, q->nazivreda);
	q->queuerecvfromserv = queueCreate(1, q->nazivreda);
	char* niz = (char*)malloc(4);
	//char niz2[] = "123";
	//memcpy(niz, niz2, 4);
	//enq(q->queuerecvfromserv, niz);
	//q->handle = 0;
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

	/* patch out first element */
	e = q->head;
	q->head = e->next;

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

void ParseQueuePairNames(char* c)
{

	char* token = strtok(c, ",");

	int br = 0;

	strings[br] = (char*)malloc(strlen(token));
	strcpy(strings[br], token);

	br++;

	// loop through the string to extract all other tokens
	while (token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, ",");
		if (token != NULL)
		{
			strings[br] = token;
			br++;
		}
	}
}
/*
	Funkcija kao parametre prima ID servera (ili klijenta), soket za komunikaciju, i pokazivac na listu podignutih servera, ili klijenata.
	Memorija se zauzima dinamicki za svaki novi elemenat liste upotrebom funkcije malloc,
	i redom se popunjavaju polja novog elementa liste, kao i povezivanje novog elementa sa starim (postavljanje na kraj),
	ili njegovo postavljanje na pocetak liste.
	Soket za komunikaciju se koristi da bi proksi znao kojem klijentu/serveru prosledjuje poruke.
*/
void ListAdd(char* c, SOCKET s, DWORD id, HANDLE h, List** head)
{
	List* el;
	el = (List*)malloc(sizeof(List));
	el->num = 0;
	queuepair* qpr = queuePairCreate(c);

	el->s = s;
	el->threadID = id;
	el->queuepair = qpr;
	el->clienth = h;
	el->next = NULL;
	el->ready = 1;

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

//void ListAdd(int number, queuepair* qpr, List** head)
//{
//	List* el;
//	el = (List*)malloc(sizeof(List));
//	//el->num = number;
//	//el->s = s;
//	//el->threadID = id;
//	//el->clienth = h;
//	el->next = NULL;
//	el->ready = 0;
//	el->queuepair = qpr;
//	if (*head == NULL) {
//		*head = el;
//	}
//	else {
//		List* temp = *head;
//		while (temp->next != NULL) {
//			temp = temp->next;
//		}
//		temp->next = el;
//	}
//}
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

//funkcija vraca sve nazive redova
char* GetAllQueuePairNames(List* head)
{
	List* temp = head;

	if (temp != NULL)
	{
		char* buffer = (char*)malloc(sizeof(temp->queuepair->nazivreda) + 1);
		strcpy(buffer, temp->queuepair->nazivreda);
		char x[2] = ","; //posle svakog naziva se stavlja zarez
		buffer = strcat(buffer, x);
		//printf(buffer);
		while (temp->next != NULL)
		{
			temp = temp->next;
			buffer = (char*)realloc(buffer, sizeof(temp->queuepair->nazivreda) + 1);
			strcat(buffer, temp->queuepair->nazivreda);
			strcat(buffer, x);

		}
		buffer[strlen(buffer) - 1] = 0; //na kraj se stavi \0 karakter da obrise poslednji zarez

		return buffer;
	}
	return NULL;
}

char* GetFreeQueuePairNames(List* head)
{
	/////////////////////////////////////////
	return NULL;
}


List* ListElementAt(char* naziv, List* head)
{
	char str[50] = "";
	strcpy(str, naziv);
	if (ListCount(head) > 0) {
		List* temp = head;

		while (temp != NULL) {
			if (strcmp(str, temp->queuepair->nazivreda) == 0)
			{
				return temp;
			}
			temp = temp->next;
		}
		return NULL;
	}
	return NULL;
}

/*
	Funkcija select omogucava pracenje stanja pisanja, citanja, i gresaka na jednom ili vise soketa.
	Parametri funkcije su soket koji se obradjuje i promenljiva read; u slucaju da se proverava mogucnost citanja
	ovaj parametar ce biti postavljen na true (operacije recv, recvfrom, accept, close);
	ukoliko je taj parametar false vrsi se provera mogucnosti pisanja (operacije send, sendto, connect).
	U while petlji funkcije vrsi se inicijalizacija seta, dodavanje desktriptora soketa u set, podesavanje
	maksimalnog dozvoljenog vremena koje ce funkcija select cekati da se neki od dogadjaja desi, i na kraju
	pozivanje implementirane funkcije select sa odgovarajucim parametrima, u zavisnosti da li se podaci salju ili primaju.
*/
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

DWORD WINAPI ClientServerCommunicateThread(LPVOID lpParam)
{

	printf("ClientServ Thread\n");
	int nDataLength;
	char* buffer = (char*)calloc(1024, sizeof(char));
	//ThreadParams* socket = (ThreadParams*)lpParam;
	char* naziv = (char*)lpParam;
	List* el = ListElementAt(naziv, lista);
	//printf("%s", el->queuepair->nazivreda);
	Select(el->s, true);


	while (true)
	{
		while ((nDataLength = recv(el->s, buffer, sizeof(buffer), 0)) > 0) {
			if (buffer != "exit")
			{
				printf("Client with thread ID: %d, Sent: %s, to QueuePair: %s", el->threadID, buffer, el->queuepair->nazivreda);
				putchar('\n');
				EnterCriticalSection(&cs);
				enq(el->queuepair->queuesendtoserv, buffer);
				LeaveCriticalSection(&cs);
			}
		}
		if (buffer == "exit")
			break;
	}

	//int iResult = 0;
	//int idserv = *((int*)lpParam);
	//free(lpParam);
	//char* buffer = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	//int* pok = (int*)buffer;
	//List* servis = ListElementAt(idserv, listservicehead);

	//while (true)
	//{
	//	if (servis->ugasi == 0)
	//	{
	//		if (servis->ready == 1)
	//		{
	//			SOCKET socket = servis->s;
	//			Select(socket, true);s

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
	return NULL;
}


DWORD WINAPI ClientChooseQueuePair(LPVOID lpParam)
{
	ThreadParams* socket = (ThreadParams*)lpParam;
	int nDataLength;

	char myString[] = "";
	char* buffer = (char*)calloc(1024, sizeof(char));
	int iResult = 0;

	iResult = recv(socket->clientsocket, buffer, DEFAULT_BUFLEN, 0);
	socket->naziv = buffer;
	printf("%s", buffer);

	List* el = ListElementAt(buffer, lista);
	if (el != NULL)
	{
		el->s = socket->clientsocket;

		el->ready = 1;
		HANDLE handle1;
		DWORD dw1;
		handle1 = CreateThread(NULL, 0, &ClientServerCommunicateThread, (LPVOID)socket->naziv, 0, &dw1);
		el->clienth = handle1;
		el->threadID = dw1;
	}
	else
	{
		HANDLE handle;
		DWORD dw;
		handle = CreateThread(NULL, 0, &ClientServerCommunicateThread, (LPVOID)socket->naziv, 0, &dw);
		ListAdd(buffer, socket->clientsocket, dw, handle, &lista);
	}

	unsigned long int nonBlockingMode = 1;
	iResult = ioctlsocket(socket->clientsocket, FIONBIO, &nonBlockingMode);

	if (iResult == SOCKET_ERROR)
	{
		printf("ioctlsocket failed with error: %ld\n", WSAGetLastError());
		return 1;
	}

	return NULL;
}

DWORD WINAPI ServerToServer1(LPVOID lpParam)
{
	char buffer[DEFAULT_BUFLEN];
	printf("1");
	int iResult;
	Select(serverSocket, 0);
	while (true)
	{
		char* queuepairnames = GetAllQueuePairNames(lista);
		if (queuepairnames != NULL)
			iResult = send(serverSocket, queuepairnames, strlen(queuepairnames), 0);
		else
			iResult = send(serverSocket, ".i.", 4, 0);
		Select(serverSocket, 1);
		iResult = recv(serverSocket, buffer, DEFAULT_BUFLEN, 0);
		printf("%s", buffer);

		Sleep(5000);
	}
	return NULL;
}


DWORD WINAPI ServerToServer2(LPVOID lpParam)
{
	char* buffer = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));

	printf("2");
	int iResult;
	while (true)
	{
		Select(serverSocket, 1);
		iResult = recv(serverSocket, buffer, DEFAULT_BUFLEN, 0);
		if (strlen(buffer) == 0)
		{
			printf("nista");
		}
		else if (buffer[0] == 'X')
		{
			memmove(buffer, buffer + 1, strlen(buffer));
			ParseQueuePairNames(buffer);
		}
		else
		{
			printf("%s", buffer);
		}
		Select(serverSocket, 0);
		char* queuepairnames = GetAllQueuePairNames(lista);
		if (queuepairnames != NULL)
			iResult = send(serverSocket, queuepairnames, strlen(queuepairnames), 0);
		else
			iResult = send(serverSocket, "", 1, 0);

		memset(buffer, 0, DEFAULT_BUFLEN);
	}
	return NULL;
}
