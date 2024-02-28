CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto -lm

all: server simpleClient

server:  server.c base64.c
	$(CC) $(CFLAGS) -o  server server.c base64.c  $(LDFLAGS)

simpleClient: simpleClient.c
	$(CC) $(CFLAGS) -o  simpleClient simpleClient.c

clean:
	rm -f server simpleClient
