/* A simple server in the internet domain using TCP
   The port number is passed as an argument */
#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>

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
        return 0;
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
        return 0;
    }

}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd1,newsockfd2, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;

    checkPort(argc);
    sockfd = serverAssignSocket();
    portno = atoi(argv[1]);
    serverBind(sockfd,portno);

    newsockfd1 = serverAccept(sockfd);
    newsockfd2 = serverAccept(sockfd);

    while (1){
        serverRead(newsockfd1,buffer);
        serverWrite(newsockfd2,buffer);

        serverRead(newsockfd2,buffer);
        serverWrite(newsockfd1,buffer);
    }
}