#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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
    printf("Please enter the message: ");
    bzero(buffer,256);
    fgets(buffer,255,stdin);
    n = write(sockfd,buffer,strlen(buffer));
    if (n < 0) 
            error("ERROR writing to socket");

    if(memcmp(buffer,"bye",strlen("bye")) == 0)
    {
        return 0;
    }
}

void clientRead(int sockfd,char buffer[])
{
    int n;
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0) 
            error("ERROR reading from socket");
    printf("Here is the message: %s",buffer);
    if(memcmp(buffer,"bye",strlen("bye")) == 0)
    {
        return 0;
    }
}

int main(int argc, char *argv[])
{
    int sockfd, portno, n;
    char buffer[256];

    init(argc,argv);
    
    portno = atoi(argv[2]);
    sockfd = clientAssignSocket();
    clientConnectToServer(argv,portno,sockfd);

    while(1){
        clientWrite(sockfd,buffer);
        clientRead(sockfd,buffer);        
    }
}