#include "Header.h"

CRITICAL_SECTION cs; //Globalna promenljiva, kriticna sekcija
List* lista = NULL; //lista za paroveredova
SOCKET serverSocket = INVALID_SOCKET;
char* strings[MAX_QUEUEPAIRS] = { NULL };

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
	char* poruka = (char*)calloc(1024, sizeof(char));
	int nDataLength;
	int flag = 0;
	while (true)
	{
		for (int i = 0; i < MAX_QUEUEPAIRS; i++)
		{
			if (strings[i] == NULL)
			{
				flag = 0;
				break;
			}
			else if (strcmp(strings[i], el->queuepair->nazivreda) == 0)
			{
				flag = 1;
				break;
			}
		}
		if (!queueEmpty(el->queuepair->queuesendtoserv) && flag == 1)
		{
			EnterCriticalSection(&cs);
			poruka = deq2(el->queuepair->queuesendtoserv, poruka);
			strcat(poruka, ":");
			strcat(poruka, el->queuepair->nazivreda);
			strcat(poruka, ",");
			LeaveCriticalSection(&cs);
			nDataLength = send(serverSocket, poruka, strlen(poruka), 0);
		}
		else if (flag == 0)
		{
			Sleep(1000);
		}
		else
		{
			Sleep(15);
		}
	}
	free(poruka);
	return NULL;
}

DWORD WINAPI RecvQueueThread(LPVOID lpParam)
{
	char* c = (char*)lpParam;
	List* el = ListElementAt(c, lista);
	char* poruka = (char*)calloc(1024, sizeof(char));
	int nDataLength;

	while (true)
	{
		if (el != NULL)
		{
			if (el->ready == 1 && !queueEmpty(el->queuepair->queuerecvfromserv))
			{
				EnterCriticalSection(&cs);
				poruka = deq2(el->queuepair->queuerecvfromserv, poruka);
				LeaveCriticalSection(&cs);
				Select(el->s, false);
				nDataLength = send(el->s, poruka, strlen(poruka), 0);
				if (nDataLength == -1)
				{
					printf("Disconected client");
					el->ready = 0;
					enq(el->queuepair->queuerecvfromserv, poruka);
				}
				memset(poruka, 0, strlen(poruka));
			}
		}
		Sleep(15);
	}
	free(poruka);
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
	char* c = (char*)malloc(strlen(value) + 1);
	strcpy(c, value);
	e->value = c;

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

	strcpy(poruka, q->head->value);

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

	if (strings[br] == NULL)
	{
		strings[br] = (char*)malloc(strlen(token));
		strcpy(strings[br], token);
	}
	br++;

	while (token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, ",");
		if (token != NULL)
		{
			if (strings[br] == NULL)
			{
				strings[br] = (char*)malloc(strlen(token));
				strcpy(strings[br], token);
			}
			br++;
		}
	}
}

int ParseString(char* c, char* out[])
{
	char* token = strtok(c, ",");

	int br = 0;


	out[br] = (char*)malloc(strlen(token));
	strcpy(out[br], token);

	br++;

	while (token != NULL) {
		printf("%s\n", token);
		token = strtok(NULL, ",");
		if (token != NULL)
		{

			out[br] = (char*)malloc(strlen(token));
			strcpy(out[br], token);

			br++;
		}
	}

	return br;
}


void ListAdd(char* c, SOCKET s, DWORD id, HANDLE h, List** head, int ready)
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
	el->ready = ready;

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
	int br = 1;
	if (temp != NULL)

		//QWERTTY,LOL\0
	{
		char* buffer = (char*)calloc(strlen(temp->queuepair->nazivreda) + 2, sizeof(char));
		strcpy(buffer, temp->queuepair->nazivreda);
		char zarez[2] = ","; //posle svakog naziva se stavlja zarez
		memcpy(buffer + strlen(buffer), zarez, 2);
		while (temp->next != NULL)
		{
			temp = temp->next;
			buffer = (char*)realloc(buffer, strlen(buffer) + strlen(temp->queuepair->nazivreda) + 2);
			strcat(buffer, temp->queuepair->nazivreda);
			memcpy(buffer + strlen(buffer), zarez, 2);
		}
		buffer[strlen(buffer) - 1] = '\0'; //na kraj se stavi \0 karakter da obrise poslednji zarez

		buffer = (char*)realloc(buffer, strlen(buffer) + 2);

		memmove(buffer + 1, buffer, strlen(buffer) + 1);
		buffer[0] = 'X'; // X na pocetak da bi druga strana razlikovala naziveredova od obicne poruke
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

DWORD WINAPI ServerEnqueueThread(LPVOID lpParam)
{
	printf("ClientServ Thread\n");
	int nDataLength;
	char* buffer = (char*)calloc(1024, sizeof(char));
	char* naziv = (char*)lpParam;
	List* el = ListElementAt(naziv, lista);


	while (true)
	{
		if (el == NULL)
		{
			List* el = ListElementAt(naziv, lista);
		}
		else
		{
			Select(el->s, true);
			while ((nDataLength = recv(el->s, buffer, 1023, 0)) > 0) {
				if (strcmp(buffer, "exit") != 0)
				{
					printf("Client with thread ID: %d, Sent: %s, to QueuePair: %s", el->threadID, buffer, el->queuepair->nazivreda);
					putchar('\n');
					EnterCriticalSection(&cs);
					enq(el->queuepair->queuesendtoserv, buffer);
					LeaveCriticalSection(&cs);
					memset(buffer, 0, 1024);
				}
				else
				{
					printf("exited");
					el->ready = 0;
				}
			}
		}
	}
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

	unsigned long int nonBlockingMode = 0;
	iResult = ioctlsocket(socket->clientsocket, FIONBIO, &nonBlockingMode);

	iResult = recv(socket->clientsocket, buffer, 1024, 0);
	socket->naziv = buffer;
	printf("\n%s", buffer);

	List* el = ListElementAt(buffer, lista);
	if (el != NULL)
	{
		if (el->ready == 0)
		{

			el->s = socket->clientsocket;

			el->ready = 1;
			HANDLE handle1;
			DWORD dw1;
			handle1 = CreateThread(NULL, 0, &ServerEnqueueThread, (LPVOID)socket->naziv, 0, &dw1);
			el->clienth = handle1;
			el->threadID = dw1;
		}
		else
		{
			printf("already taken");
		}
	}
	else
	{
		HANDLE handle;
		DWORD dw;
		handle = CreateThread(NULL, 0, &ServerEnqueueThread, (LPVOID)socket->naziv, 0, &dw);
		ListAdd(buffer, socket->clientsocket, dw, handle, &lista, 1);
	}

	nonBlockingMode = 1;
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
	char* buffer = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	printf("ServerToServer1\n");
	int iResult;
	while (true)
	{
		Select(serverSocket, 0);
		char* queuepairnames = GetAllQueuePairNames(lista);
		if (queuepairnames != NULL)
			iResult = send(serverSocket, queuepairnames, strlen(queuepairnames), 0);
		else
			iResult = send(serverSocket, "", 1, 0);
		Sleep(5000);
	}
	return NULL;
}


DWORD WINAPI ServerToServer2(LPVOID lpParam)
{
	char* buffer = (char*)calloc(DEFAULT_BUFLEN, sizeof(char));
	char* buffertemp;
	char* parsedstrings[50];
	//unsigned long int nonBlockingMode = 1;
	printf("2");
	int iResult;
	//iResult = ioctlsocket(serverSocket, FIONBIO, &nonBlockingMode);
	while (true)
	{
		Select(serverSocket, 1);

		//printf("%s", strings[0]);
		iResult = recv(serverSocket, buffer, DEFAULT_BUFLEN, 0);
		if (iResult > 0)
		{

			if (strlen(buffer) == 0)
			{
				printf("nista");
			}
			else if (buffer[0] == 'X')
			{
				memmove(buffer, buffer + 1, strlen(buffer)); //zbog x-a da se obrise pomeri string u desno za 1
				ParseQueuePairNames(buffer);
			}
			else
			{
				printf("Recv: %s", buffer);
				int brojac = ParseString(buffer, parsedstrings);
				for (int i = 0; i < brojac; i++)
				{
					buffertemp = (char*)malloc(strlen(parsedstrings[i]) + 1);
					strcpy(buffertemp, parsedstrings[i]);
					char* token2 = strtok(buffertemp, ":");
					token2 = strtok(NULL, ":");
					List* el = ListElementAt(token2, lista);
					EnterCriticalSection(&cs);
					enq(el->queuepair->queuerecvfromserv, buffertemp);
					LeaveCriticalSection(&cs);
				}
				//token= strtok(NULL, ",");

			}
			memset(buffer, 0, strlen(buffer));

		}
	}
	return NULL;
	return NULL;
}