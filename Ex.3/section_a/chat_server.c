
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8888
#define MAX_MSG_SIZE 1024
#define MAX_CLIENTS 10000

struct ClientInfo {
    int socket;
    pthread_t thread;
};

// Define a global array to store client sockets
int client_sockets[MAX_CLIENTS];
int num_clients = 0;
pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

// Function to send a message to all clients except the sender
void send_message_to_clients(char *message, int sender_socket) {
    // Create a buffer to hold the full message including the sender's socket number
    char full_message[MAX_MSG_SIZE + 20]; // Additional space for "client %d: " and sender_socket
    snprintf(full_message, sizeof(full_message), "client %d: %s", sender_socket, message);

    // Lock the mutex to access the client_sockets array
    pthread_mutex_lock(&clients_mutex);

    // Iterate through all client sockets and send the message
    for (int i = 0; i < num_clients; i++) {
        if (client_sockets[i] != sender_socket) { // Skip the sender
            send(client_sockets[i], full_message, strlen(full_message), 0);
        }
    }

    // Unlock the mutex
    pthread_mutex_unlock(&clients_mutex);
}

// Function to handle communication with a client
void* handle_client(void* arg) {
    struct ClientInfo* client = (struct ClientInfo*)arg;
    char buffer[MAX_MSG_SIZE];

    // Loop to continuously receive messages from the client
    while (1) {
        ssize_t recv_size = recv(client->socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Client %d disconnected.\n", client->socket);
            break;
        }
        buffer[recv_size] = '\0'; // Null terminate the received message
        printf("Client %d: %s", client->socket, buffer);

        // Send the received message to all clients
        send_message_to_clients(buffer, client->socket);
    }

    // Close the client socket
    close(client->socket);

    // Remove the client socket from the global array and update num_clients
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

    // Free the memory allocated for the client structure
    free(client);

    return NULL;
}

int main() {
    printf("Section A - Server\n");
    int server_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    // Create a socket for the server
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Initialize the server address structure
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // Bind the server socket to the specified address and port
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

    // Accept and handle incoming client connections
    while (1) {
        int client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket == -1) {
            perror("accept");
            continue;
        }

        printf("New client connected: %d\n", client_socket);

        // Allocate memory for the client information structure
        struct ClientInfo* client_info = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
        client_info->socket = client_socket;

        // Add the client socket to the global array and update num_clients
        pthread_mutex_lock(&clients_mutex);
        client_sockets[num_clients] = client_socket;
        num_clients++;
        pthread_mutex_unlock(&clients_mutex);

        // Create a new thread to handle communication with the client
        if (pthread_create(&client_info->thread, NULL, handle_client, (void*)client_info) != 0) {
            perror("pthread_create");
            close(client_socket);
            free(client_info);
        }
    }

    // Close the server socket
    close(server_socket);

    return 0;
}
