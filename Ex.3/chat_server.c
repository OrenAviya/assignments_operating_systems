#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10

struct ClientInfo {
    int socket;
    pthread_t thread;
};

// Define a global array to store client sockets
int client_sockets[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void broadcast_message(char *message, int sender_socket) {
    char full_message[MAX_MSG_SIZE + 20]; // Additional space for "client %d: " and sender_socket
    snprintf(full_message, sizeof(full_message), "client %d: %s", sender_socket, message);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_socket) {
            send(client_sockets[i], full_message, strlen(full_message), 0);
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}



void* handle_client(void* arg) {
    struct ClientInfo* client = (struct ClientInfo*)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        ssize_t recv_size = recv(client->socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Client %d disconnected.\n", client->socket);
            break;
        }
        buffer[recv_size] = '\0';
        printf("Client %d: %s", client->socket, buffer);

        broadcast_message(buffer, client->socket);
    }

    close(client->socket);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] == client->socket) {
            for (int j = i; j < num_clients - 1; j++) {
                client_sockets[j] = client_sockets[j + 1];
            }
            num_clients--;
            break;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
    free(client);

    return NULL;
}

int main() {
    printf("Section A - Server\n");
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        printf("New client connected: %d\n", client_socket);

        struct ClientInfo* client_info = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
        client_info->socket = client_socket;

        pthread_mutex_lock(&clients_mutex);
        client_sockets[num_clients] = client_socket;
        num_clients++;
        pthread_mutex_unlock(&clients_mutex);

        if (pthread_create(&client_info->thread, NULL, handle_client, (void*)client_info) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(client_info);
        }
    }

    close(server_socket);

    return 0;
}
