CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto

all: server client

server: mysimplehttpServer.c
	$(CC) $(CFLAGS) -o  server mysimplehttpServer.c $(LDFLAGS)

client: SimpleClient.c
	$(CC) $(CFLAGS) -o  client SimpleClient.c

clean:
	rm -f server client
