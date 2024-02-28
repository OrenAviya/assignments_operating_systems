CC = gcc
CFLAGS = -Wall -Wextra -Werror
LDFLAGS = -lpthread -lssl -lcrypto -lm

all: server client complexClient

server:  server.c Base64Decode.c Base64Encode.c
	$(CC) $(CFLAGS) -o  server mysimplehttpServer.c Base64Decode.c Base64Encode.c  $(LDFLAGS)

client: SimpleClient.c
	$(CC) $(CFLAGS) -o  client SimpleClient.c

complexClient: complexClient.c
	$(CC) $(CFLAGS) -o  complexClient complexClient.c $(LDFLAGS)

clean:
	rm -f server client complexClient
