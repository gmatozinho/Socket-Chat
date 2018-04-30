CC=gcc 
CFLAGS=-Wall

all:	server	client

server:	server.c
	gcc -o server server.c -lpthread -ansi

client:	client.c
	gcc -o client client.c -lpthread -ansi

clean:	cleanServer	cleanClient 

runclient:
	./client localhost 10000
cleanServer :
	rm -f server

cleanClient :
	rm -f client

