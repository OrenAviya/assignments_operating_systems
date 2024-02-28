#Makefile for the entire project

all: chat_client chat_server libserver

# Build the proactor library
libproactor.a: proactor.o
	ar rcs libproactor.a proactor.o

proactor.o: proactor.c proactor.h
	gcc -c -o proactor.o proactor.c -pthread

# Build the chat client
chat_client: chat_client.c libproactor.a
	gcc -o chat_client chat_client.c -L. -lproactor

# Build the chat server
chat_server: chat_server.c libproactor.a
	gcc -o chat_server chat_server.c -L. -lproactor

# Build the server with proactor
libserver: server_proactor.o libproactor.a
	gcc -o libserver server_proactor.o -L. -lproactor -pthread

server_proactor.o: server_proactor.c
	gcc -c -o server_proactor.o server_proactor.c -I.

clean:
	rm -f chat_client chat_server libserver libproactor.a *.o
