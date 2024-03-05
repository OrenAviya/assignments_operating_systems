#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h> // Include pthread.h for mutex functions
#include "../section_b/proactor.h"

#define PORT 8888
#define MAX_CLIENTS 10
#define MAX_MSG_SIZE 1024

// Structure to represent each connected client
struct Client {
    int socket;
    struct Client* next;
};

// Global variables for managing client connections
static struct Client* clients = NULL;
static pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;
static int num_clients = 0; // Track the number of clients

// Function to broadcast message to all clients except the sender
void broadcast_message(const char* message, int sender_socket) {
    char full_message[MAX_MSG_SIZE + 20]; // Additional space for "Client %d: " and sender_socket
    snprintf(full_message, sizeof(full_message), "Client %d: %s", sender_socket, message);
    pthread_mutex_lock(&clients_mutex);
    struct Client* current = clients;
    while (current != NULL) {
        if (current->socket != sender_socket) {
            send(current->socket, full_message, strlen(full_message), 0);
        }
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);
}

// Function to handle client connections
void handle_connection(int socket) {
    // Add the client to the list of connected clients
    struct Client* new_client = (struct Client*)malloc(sizeof(struct Client));
    new_client->socket = socket;
    new_client->next = clients;
    clients = new_client;
    num_clients++;

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

        // Broadcast message to all other clients
        broadcast_message(buffer, socket);
    }

    // Remove the client from the list of connected clients
    pthread_mutex_lock(&clients_mutex);
    struct Client* prev = NULL;
    struct Client* current = clients;
    while (current != NULL) {
        if (current->socket == socket) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                clients = current->next;
            }
            free(current);
            num_clients--;
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&clients_mutex);

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
