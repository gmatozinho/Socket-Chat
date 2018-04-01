CC=gcc 
CFLAGS=-Wall

chat :
	gcc -o client client.c -lpthread
	gcc -o server server.c -lpthread


clean :
	rm server *.o
	rm client *.o