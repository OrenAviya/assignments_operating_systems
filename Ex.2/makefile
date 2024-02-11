CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto

all: server client complexClient

server: mysimplehttpServer.c
	$(CC) $(CFLAGS) -o  server mysimplehttpServer.c $(LDFLAGS)

client: SimpleClient.c
	$(CC) $(CFLAGS) -o  client SimpleClient.c

complexClient: complexClient.c
	$(CC) $(CFLAGS) -o  complexClient complexClient.c $(LDFLAGS)

clean:
	rm -f server client complexClient
