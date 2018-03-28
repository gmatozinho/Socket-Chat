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

int main(int argc, char *argv[])
{
    int sockfd, newsockfd1,newsockfd2, portno;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n;
    // if (argc < 2) {
    //     fprintf(stderr,"ERROR, no port provided\n");
    //     exit(1);
    // }
    checkPort(argc);
    sockfd = serverAssignSocket();
    portno = atoi(argv[1]);
    serverBind(sockfd,portno);



    // sockfd = socket(AF_INET, SOCK_STREAM, 0);
    // if (sockfd < 0) 
    // error("ERROR opening socket");


    // bzero((char *) &serv_addr, sizeof(serv_addr));
    // portno = atoi(argv[1]);
    // serv_addr.sin_family = AF_INET;
    // serv_addr.sin_addr.s_addr = INADDR_ANY;
    // serv_addr.sin_port = htons(portno);
    // if (bind(sockfd, (struct sockaddr *) &serv_addr,
    //         sizeof(serv_addr)) < 0) 
    //         error("ERROR on binding");
    newsockfd1 = serverAccept(sockfd);
    newsockfd2 = serverAccept(sockfd);

        // listen(sockfd,5);
        // clilen = sizeof(cli_addr);
        // newsockfd1 = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        // newsockfd2 = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);

        // if (newsockfd1 < 0)
        //     error("ERROR on accept");
        // if(newsockfd2 < 0)
        //     error("ERRO on accept");

        while (1)
    {
        bzero(buffer,256);
        n = read(newsockfd1,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        // printf("Here is the message: %s",buffer);
        if(memcmp(buffer,"bye",strlen("bye")) ==0)
        {
            return 0;
        }
        //printf("Please enter the message: ");
        //bzero(buffer,256);
        //fgets(buffer,255,stdin);
        n = write(newsockfd2,buffer,strlen(buffer));
        if (n < 0) 
                error("ERROR writing to socket");
        if(memcmp(buffer,"bye",strlen("bye")) == 0)
        {
            close(newsockfd1);
            close(newsockfd2);
            return 0;
        }
        
        bzero(buffer,256);
        n = read(newsockfd2,buffer,255);
        if (n < 0) error("ERROR reading from socket");
        // printf("Here is the message: %s",buffer);
        if(memcmp(buffer,"bye",strlen("bye")) ==0)
        {
            close(newsockfd1);
            close(newsockfd2);
            return 0;
        }
        //printf("Please enter the message: ");
        //bzero(buffer,256);
        //fgets(buffer,255,stdin);
        n = write(newsockfd1,buffer,strlen(buffer));
        if (n < 0) 
                error("ERROR writing to socket");
        if(memcmp(buffer,"bye",strlen("bye")) == 0)
        {
            return 0;
        }
        //return 0; 
    }
}