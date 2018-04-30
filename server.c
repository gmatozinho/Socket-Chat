/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

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

void serverSendBroadcast(char *buffer){
    int i;
    int sockfd;
    
    for (i = 0; i < t_count; i++){
        sockfd = sockets[i];
        
		if (sockfd != -1){
            serverWrite(sockfd,buffer);
		}
    }
}

void serverCloseBroadcast(char *buffer){
    int i;
    int sockfd;
    
    for (i = 0; i < t_count; i++){
        sockfd = sockets[i];
        
		if (sockfd != -1){
            printf("\nKilling client %d\n", sockfd);	
            serverWrite(sockfd,buffer);
		}        
    }
}

void closeAll()
{   
    char buffer[10];
    strcpy(buffer,"bye");
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
    serverSendBroadcast(buffer);
    printf("%d sent: %s \n", sockfd, buffer);
}

int findSocketToClose(int sockfd)
{
    int i;
    for (i = 0; i < 5; i++){
    	if (sockets[i] == sockfd) {
    		return i;
    	}
    }
    
}

void closeSocket(int position)
{
    pthread_mutex_lock(&mutex);
    sockets[position] = -1;
    pthread_mutex_unlock(&mutex);
}

void nomesinal(int sinal, char* str)
{
	switch(sinal)
	{
      	case 2:  
            strcpy(str, "SIGINT");  
            break;
		case 15:  
            strcpy(str, "SIGTERM"); 
            break;
	}
}

void tratasinal(int sinal)
{ 
    if(sinal==2 || sinal == 15){
        closeAll();
    }
}

void *serverListener(void *socket){
    char buffer[256];
    int sockfd, n;
    int position, i;
    
    sockfd = *(int *)socket;
    bzero(buffer,256);
    
    signal(2, tratasinal);
	signal(15, tratasinal);

    while (memcmp(buffer,"bye",strlen("bye"))) {        
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
	int first_empty,i;
    first_empty = -1;
		
    pthread_mutex_lock(&mutex);
    for (i = 0; i < 5; i++){
        if (sockets[i] == -1) {
            first_empty = i;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);

    return first_empty;    
}

void serverAcceptClient(int sockfd,int first_empty)
{
    int newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
    if (newsockfd < 0) 
        error("ERROR on accept");
    
    pthread_create(&threads[first_empty], NULL, serverListener, &newsockfd);
    sockets[first_empty] = newsockfd;

    t_count++;
    printf("Connected: %d \n", t_count);
}

void *getClients(void *socket){
	int first_empty, clilen, newsockfd, i, sockfd;
	struct sockaddr_in cli_addr;
	
	sockfd = *(int *)socket;
	
	do {
		listen(sockfd,5);		
        first_empty = getFirstEmpty();
		
		if (first_empty != -1){
			serverAcceptClient(sockfd,first_empty);
		}
		else printf("Server is full, try again later\n");
        
        
    } while (t_count > 0);
	
	return NULL;
}

void startSocketVector()
{
    int i=0;
    for (i = 0; i < 5; i++){
		sockets[i] = -1;
    }
}

void mallocs()
{
    threads = malloc(5 * sizeof(pthread_t));
    sockets = malloc(5 * sizeof(int));
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

    pthread_create(&main_thread, NULL, getClients, &sockfd);	
	pthread_join(main_thread, NULL);

    free(threads);
	free(sockets);
     
    close(sockfd);

    return 0;
}