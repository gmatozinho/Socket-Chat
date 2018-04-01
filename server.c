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


int* sockets;
pthread_t main_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int t_count = 0;
pthread_t* threads;

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


int serverAccept(int sockfd)
{
    int newsockfd, clilen;
    struct sockaddr_in serv_addr, cli_addr;

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);

    if (newsockfd < 0){
        error("ERROR on accept");
    };

    return newsockfd;
}

void serverRead(int newsockfd,char buffer[])
{
    int n;

    bzero(buffer,256);
    n = read(newsockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    if(memcmp(buffer,"bye",strlen("bye")) ==0)
    {

        close(newsockfd);
        return;
    }
}

void serverWrite(int newsockfd,char buffer[])
{
    int n;
    n = write(newsockfd,buffer,strlen(buffer));
    if (n < 0) 
            error("ERROR writing to socket");
    if(memcmp(buffer,"bye",strlen("bye")) == 0)
    {

        close(newsockfd);
        return;
    }

}

void *ouvinte(void *socket){
    char buffer[256];
    int sockfd, n;
    int position, i;
    
    sockfd = *(int *)socket;
    bzero(buffer,256);
    
    while (strcmp(buffer, "bye") != 0) {
        bzero(buffer,256);
        n = read(sockfd,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        broadcast(buffer);
        printf("%d sent: %s \n", sockfd, buffer);
    }
    
    for (i = 0; i < 5; i++){
    	if (sockets[i] == sockfd) {
    		position = i;
    		break;
    	}
    }
    
    pthread_mutex_lock(&mutex);
    sockets[position] = -1;
    pthread_mutex_unlock(&mutex);
    
    t_count--;
    //close(sockfd);
    
    printf("Killing client %d \n", sockfd);
	
	if (t_count == 0) pthread_cancel(main_thread);

    return NULL;
}

void *get_clients(void *socket){
	int first_empty, clilen, newsockfd, i, sockfd;
	struct sockaddr_in cli_addr;
	
	sockfd = *(int *)socket;
	
	do {
		listen(sockfd,5);
		
		//Pega a primeira posicao disponivel no vetor
		first_empty = -1;
		
		pthread_mutex_lock(&mutex);
		for (i = 0; i < 5; i++){
			if (sockets[i] == -1) {
				first_empty = i;
				break;
			}
		}
		pthread_mutex_unlock(&mutex);
		
		if (first_empty != -1){ //se ainda ha posicoes disponiveis no vetor
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			if (newsockfd < 0) 
				error("ERROR on accept");
			
			pthread_create(&threads[first_empty], NULL, ouvinte, &newsockfd);
			sockets[first_empty] = newsockfd;

			t_count++;
			printf("Connected: %d \n", t_count);
		}
		else printf("Server is full, try again later\n");
        
        
    } while (t_count > 0);
	
	return NULL;
}



void broadcast(char *buffer){
    int i;
    int sockfd, n;
    
    for (i = 0; i < t_count; i++){
        sockfd = sockets[i];
        
		if (sockfd != -1){
			n = write(sockfd, buffer, strlen(buffer));
			if (n < 0) error("ERROR writing to socket");
		}
    }
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd1,newsockfd2, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    int i;

    threads = malloc(5 * sizeof(pthread_t));
    sockets = malloc(5 * sizeof(int));

    checkPort(argc);
    sockfd = serverAssignSocket();
    portno = atoi(argv[1]);
    serverBind(sockfd,portno);

   /*  newsockfd1 = serverAccept(sockfd);
    newsockfd2 = serverAccept(sockfd); */

    //Inicializando vetor de sockets para representar que nao ha clientes
    for (i = 0; i < 5; i++){
		sockets[i] = -1;
    }

    for(i=0;i<5;i++)
    {
        printf("%d:\n",sockets[i]);
    }

    pthread_create(&main_thread, NULL, get_clients, &sockfd);	
	pthread_join(main_thread, NULL);

    free(threads);
	free(sockets);
     
    close(sockfd);

    return 0;

   /*  while (1){
        serverRead(newsockfd1,buffer);
        serverWrite(newsockfd2,buffer);

        serverRead(newsockfd2,buffer);
        serverWrite(newsockfd1,buffer);
    } */
}