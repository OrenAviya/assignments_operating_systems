//
// Created by aviyaob on 2/25/24.
//

// chat_server.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10

// Structure to hold client information
struct ClientInfo {
    int socket;
    pthread_t thread;
};

// Function to handle communication with a client
void* handle_client(void* arg) {
    struct ClientInfo* client = (struct ClientInfo*)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        // Receive message from client
        ssize_t recv_size = recv(client->socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        // Display message from client
        buffer[recv_size] = '\0';
        printf("Client %d: %s", client->socket, buffer);
    }

    // Close socket and free client structure
    close(client->socket);
    free(client);

    return NULL;
}

int main() {
    printf("Section A - Server\n");
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create socket
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind socket
    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_socket, MAX_CLIENTS) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while (1) {
        // Accept incoming connection
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        printf("New client connected: %d\n", client_socket);

        // Create a structure to hold client information
        struct ClientInfo* client_info = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
        client_info->socket = client_socket;

        // Create a thread to handle communication with the client
        if (pthread_create(&client_info->thread, NULL, handle_client, (void*)client_info) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(client_info);
        }
    }

    // Close server socket
    close(server_socket);

    return 0;
}

