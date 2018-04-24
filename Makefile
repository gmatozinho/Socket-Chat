CC=gcc 
CFLAGS=-Wall

all:	server	client

server:	server.c
	gcc -o server server.c -lpthread

client:	client.c
	gcc -o client client.c -lpthread

clean:	cleanServer	cleanClient 

cleanServer :
	rm -f server

cleanClient :
	rm -f client

