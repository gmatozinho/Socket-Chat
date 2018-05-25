/* A simple server in the internet domain using TCP
   The port number is passed as an argument 
    and management exchange of messages between clients 
    Accepts up to 5 clients   
*/
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

const double SIZE = 5;

int* sockets;
pthread_t main_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int t_count = 0;
pthread_t* threads;
int thisSocket;

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void checkPort(int port)
{
    if (port < 2)
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }
}

int serverAssignSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
    }

    return sockfd;
}

void serverBind(int sockfd, int portno)
{ 
    struct sockaddr_in serv_addr;

    bzero((char *)&serv_addr, sizeof(serv_addr));    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *)&serv_addr,
        sizeof(serv_addr)) < 0)
        error("ERROR on binding");
}


void serverWrite(int sockfd,char buffer[])
{
    int n;

    n = write(sockfd, buffer, strlen(buffer));
    if (n < 0) error("ERROR writing to socket");

}

void serverSendBroadcast(char *buffer, int readsocket){
    int i;
    int sockfd;
    
    for (i = 0; i < t_count; i++){
        sockfd = sockets[i];
        
		if (sockfd != -1 && sockfd != readsocket){
            serverWrite(sockfd,buffer);
		}
    }
}

void closeSocket(int position)
{
    pthread_mutex_lock(&mutex);
    sockets[position] = -1;
    pthread_mutex_unlock(&mutex);
}

void serverCloseBroadcast(char *buffer){
    int i;
    int sockfd;
    
    for (i = 0; i <= SIZE; i++){
        sockfd = sockets[i];
        
		if (sockfd != -1){
            printf("\nKilling client %d\n", sockfd);	
            serverWrite(sockfd,buffer);
            closeSocket(i);
		}        
    }    
    
}

void closeAll()
{   
    char buffer[15];
    strcpy(buffer,"server close");
    serverCloseBroadcast(buffer);
    free(threads);
	free(sockets);
    pthread_cancel(main_thread);    
    close(thisSocket);    
}

void serverRead(int sockfd,char buffer[])
{
    int n;

    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");

    if(memcmp(buffer,"bye",strlen("bye")) != 0)
    {
        char str[12];
        sprintf(str, "%d", sockfd);
        char newBuffer[255];
        strcpy(newBuffer, str);
        strcat(newBuffer," sent: ");
        strcat(newBuffer, buffer);
        serverSendBroadcast(newBuffer, sockfd);
    }
}

int findSocketToClose(int sockfd)
{
    int i;
    for (i = 0; i <= SIZE; i++){
    	if (sockets[i] == sockfd) {
    		return i;
    	}
    }    
}

void treatSignal(int signal)
{ 
    if(signal==2 || signal == 15){
        closeAll();
    }
}

void *serverListener(void *socket){
    char buffer[256];
    int sockfd, n;
    int position, i;
    
    sockfd = *(int *)socket;
    bzero(buffer,256);    
    
    while (memcmp(buffer,"bye",strlen("bye")) != 0) {        
        serverRead(sockfd, buffer);        
    }
    
    position = findSocketToClose(sockfd);
    closeSocket(position);    
    t_count--;

    printf("Killing client %d \n", sockfd);	
	if (t_count == 0) pthread_cancel(main_thread);

    return NULL;
}

int getFirstEmpty()
{
	int empty,i;
    empty = -1;
		
    pthread_mutex_lock(&mutex);
    for (i = 0; i <= SIZE; i++){
        if (sockets[i] == -1) {
            empty = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return empty;    
}

void serverAcceptClient(int sockfd,int empty)
{
    int newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");
    
    pthread_create(&threads[empty], NULL, serverListener, &newsockfd);
    sockets[empty] = newsockfd;

    t_count++;
    printf("Connected: %d \n", t_count);
}

void *getClients(void *socket){
	int empty, clilen, newsockfd, i, sockfd;
	struct sockaddr_in cli_addr;
	
	sockfd = *(int *)socket;
	
	do {
		listen(sockfd,5);		
        empty = getFirstEmpty();
		
		if (empty != -1){
			serverAcceptClient(sockfd,empty);
		}
		else printf("Server is full, try again later\n");
        
        
    } while (t_count > 0);
	
	return NULL;
}

void startSocketVector()
{
    int i=0;
    for (i = 0; i <= SIZE; i++){
		sockets[i] = -1;
    }
}

void mallocs()
{
    threads = malloc(SIZE * sizeof(pthread_t));
    sockets = malloc(SIZE * sizeof(int));
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd1,newsockfd2, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int i;

    mallocs();
    checkPort(argc);
    sockfd = serverAssignSocket();
    thisSocket = sockfd;
    portno = atoi(argv[1]);
    serverBind(sockfd,portno);
    startSocketVector();	

    signal(2, treatSignal);
	signal(15, treatSignal);

    pthread_create(&main_thread, NULL, getClients, &sockfd);	
	pthread_join(main_thread, NULL);

    free(threads);
	free(sockets);
     
    close(sockfd);

    return 0;
}