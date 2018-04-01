CC=gcc 
CFLAGS=-Wall

chat :
	gcc -o client client.c -lpthread
	gcc -o server server.c -lpthread


cleanServer :
	rm server *.o

cleanClient :
	rm client *.o