CC = gcc
CFLAGS = -Wall -Wextra -pthread

all: chat_server chat_client

chat_server: chat_server.c
	$(CC) $(CFLAGS) chat_server.c -o chat_server

chat_client: chat_client.c
	$(CC) $(CFLAGS) chat_client.c -o chat_client

clean:
	rm -f chat_server chat_client