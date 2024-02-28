CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto -lm

all: server simpleClient

server:  server.c Base64Decode.c Base64Encode.c
	$(CC) $(CFLAGS) -o  server server.c Base64Decode.c Base64Encode.c  $(LDFLAGS)

simpleClient: simpleClient.c
	$(CC) $(CFLAGS) -o  simpleClient simpleClient.c

clean:
	rm -f server simpleClient
