CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto -lm

all: server asyncClient

server:  server.c base64.c
	$(CC) $(CFLAGS) -o  server server.c base64.c $(LDFLAGS)

asyncClient: asyncClient.c
	$(CC) $(CFLAGS) -o  asyncClient asyncClient.c $(LDFLAGS)

clean:
	rm -f server asyncClient
