/* A simple client in the internet domain using localhost server
    and sending and receiving messages asynchronously 
    The port number and localhost is passed as an argument    
*/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define h_addr h_addr_list[0] /* for backward compatibility */

pthread_t listener, writer;
int serverSocket;

void error(char *msg)
{
    perror(msg);
    exit(0);
}

void init(int args,char **argv)
{
    if (args < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
}

int clientAssignSocket()
{
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        error("ERROR opening socket");
    }

    return sockfd;
}

void clientConnectToServer(char **argv, int portno, int sockfd)
{
    struct sockaddr_in serv_addr;
    struct hostent *server;

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(portno);
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0) 
        error("ERROR connecting");
}

void clientWrite(int sockfd,char buffer[])
{
    int n;
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer, strlen(buffer)-1);
    if (n < 0) error("ERROR reading from socket");    
}

void clientRequireClose()
{
    int n;
    n = write(serverSocket,"bye",3);
    if (n < 0) error("ERROR reading from socket"); 
    pthread_cancel(listener);
    pthread_cancel(writer);
    close(serverSocket);
}

void clientRead(int sockfd,char buffer[])
{
    int n;    
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) error("ERROR reading from socket");
    printf("%s\n",buffer);
}

void treatSignal(int sinal)
{
	if(sinal==2 || sinal==15){
        clientRequireClose();
    }
}

void *clientListener(void *socket){
    char buffer[256];
    int sockfd, n;    

    sockfd = *(int *)socket;
    
    bzero(buffer,256);    
    while (memcmp(buffer,"server close",strlen("server bye")) != 0) {
        clientRead(sockfd,buffer);
    }
    
    pthread_cancel(writer);
    pthread_cancel(listener);
    return NULL;
}

void *clientWriter(void *socket){
    char buffer[256];
    int sockfd, n;

    signal(2, treatSignal);
	signal(15, treatSignal);
    
    sockfd = *(int *)socket;
    bzero(buffer,256);
    
    while (memcmp(buffer,"bye",strlen("bye")) != 0 && memcmp(buffer,"server close",strlen("server bye")) != 0) {
        clientWrite(sockfd,buffer);
    }

    pthread_cancel(writer);    
    pthread_cancel(listener);
    return NULL;
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char buffer[256];

    init(argc,argv);
    
    portno = atoi(argv[2]);
    sockfd = clientAssignSocket();
    serverSocket = sockfd;
    clientConnectToServer(argv,portno,sockfd);    

    pthread_create(&listener, NULL, clientListener, &sockfd);
    pthread_create(&writer, NULL, clientWriter, &sockfd);
    
    pthread_join(listener, NULL);
    pthread_join(writer, NULL);
    
    close(sockfd);
   
    return 0;
}