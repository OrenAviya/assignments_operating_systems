//
// Created by aviyaob on 2/25/24.
//
// chat_client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define SERVER_IP "127.0.0.1"
#define PORT 8888
#define MAX_MSG_SIZE 1024

// Structure to hold client information
struct ClientInfo {
    int socket;
    pthread_t server_thread;
    pthread_t keyboard_thread;
};

// Function to handle communication with the server
void* handle_server(void* arg) {
    struct ClientInfo* client = (struct ClientInfo*)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        // Receive message from server
        ssize_t recv_size = recv(client->socket, buffer, sizeof(buffer), 0);
        if (recv_size <= 0) {
            printf("Server disconnected.\n");
            break;
        }

        // Display message from server
        buffer[recv_size] = '\0';
        printf("\n%s", buffer);

        // Print prompt for user input
        printf("Enter your message: ");
        fflush(stdout); // Ensure the prompt is printed immediately
    }

    return NULL;
}




// Function to handle keyboard input
void* handle_keyboard(void* arg) {
    struct ClientInfo* client = (struct ClientInfo*)arg;
    char buffer[MAX_MSG_SIZE];

    while (1) {
        printf("Enter your message: ");
        fgets(buffer, MAX_MSG_SIZE, stdin);

        // Send message to server
        send(client->socket, buffer, strlen(buffer), 0);
    }

    return NULL;
}

int main() {
    printf("Section A - Client\n");
    int client_socket;
    struct sockaddr_in server_addr;

    // Create socket
    if ((client_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Set up server address struct
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    server_addr.sin_port = htons(PORT);

    // Connect to server
    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Connected to server at %s:%d\n", SERVER_IP, PORT);

    // Create a struct to hold client information
    struct ClientInfo* client_info = (struct ClientInfo*)malloc(sizeof(struct ClientInfo));
    client_info->socket = client_socket;

    // Create a thread to handle communication with the server
    if (pthread_create(&client_info->server_thread, NULL, handle_server, (void*)client_info) != 0) {
        perror("pthread_create");
        close(client_socket);
        free(client_info);
        exit(EXIT_FAILURE);
    }

    // Create a thread to handle keyboard input
    if (pthread_create(&client_info->keyboard_thread, NULL, handle_keyboard, (void*)client_info) != 0) {
        perror("pthread_create");
        close(client_socket);
        free(client_info);
        exit(EXIT_FAILURE);
    }

    // Wait for both threads to finish
    pthread_join(client_info->server_thread, NULL);
    pthread_join(client_info->keyboard_thread, NULL);

    // Close client socket
    close(client_socket);

    // Free client_info structure
    free(client_info);

    return 0;
}