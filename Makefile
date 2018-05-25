CC=gcc 
CFLAGS=-Wall -ansi 

all:	server	client

server:	server.c
	${CC} ${CFLAGS} -o server server.c -lpthread 

client:	client.c
	${CC} ${CFLAGS} -o client client.c -lpthread

clean:	cleanServer	cleanClient 

runserver:
	./server 10000

runclient:
	./client localhost 10000

cleanServer :
	rm -f server

cleanClient :
	rm -f client

