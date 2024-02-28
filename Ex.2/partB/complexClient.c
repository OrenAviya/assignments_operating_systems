//
// Created by ofir on 2/28/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>

#define BUFFER_SIZE 1024

void send_request(int socket, const char *request);

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <POST/GET> <file_name/.list> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *request_type = argv[1];
    const char *file_name = argv[2];
    int server_port = atoi(argv[3]);

    const char *server_address = "127.0.0.1";

    // Create client socket
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Connect to the server
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_pton(AF_INET, server_address, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0) {
        perror("Connection failed");
        exit(EXIT_FAILURE);
    }

    // Construct the request
    char request[BUFFER_SIZE];
    snprintf(request, BUFFER_SIZE, "%s %s", request_type, file_name);

    // Send the request to the server
    send_request(client_socket, request);

    // Receive and print server response
    char response[BUFFER_SIZE];
    ssize_t bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
    if (bytes_received == -1) {
        perror("Error receiving response");
    } else {
        response[bytes_received] = '\0';
        printf("Server response: %s\n", response);
    }

    // Close the socket
    close(client_socket);

    return 0;
}

void send_request(int socket, const char *request) {
    if (send(socket, request, strlen(request), 0) == -1) {
        perror("Error sending request");
        exit(EXIT_FAILURE);
    }
}
