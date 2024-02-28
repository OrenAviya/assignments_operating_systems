//
// Created by aviyaob on 2/25/24.
//
// server_proactor.c
// server_proactor.c
#include <netinet/in.h>
#include <string.h>
#include "proactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#define PORT 8888
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 1024

void handle_connection(int socket) {
    // Handle the connection (e.g., read/write operations)
    printf("Handling connection on socket %d\n", socket);

    char buffer[MAX_MSG_SIZE];
    ssize_t recv_size;

    while (1) {
        // Receive message from client
        recv_size = recv(socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Client %d disconnected.\n", socket);
            break;
        }

        // Display message from client
        buffer[recv_size] = '\0';
        printf("Client %d: %s", socket, buffer);

    }

    // Cleanup and close the socket
    close(socket);
}

int main() {
    proactor_init();

    int server_socket;
    struct sockaddr_in server_addr;
    socklen_t client_addr_len = sizeof(struct sockaddr_in);

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
        int client_socket = accept(server_socket, (struct sockaddr*)&server_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        printf("New client connected: %d\n", client_socket);

        // Add the client socket to the proactor
        proactor_add_socket(client_socket, handle_connection);
    }

    // Cleanup
    proactor_cleanup();
    close(server_socket);

    return 0;
}
