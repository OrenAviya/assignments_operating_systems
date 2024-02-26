//
// Created by aviyaob on 2/8/24.
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 8080
#define BUFFER_SIZE 1024

void send_request(const char *request, const char *server_ip, int server_port);

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <request_type> <file_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *request_type = argv[1];
    const char *file_name = argv[2];

    printf("the file name: %s \n", file_name);

    // Construct the request based on the request type and file name
    char request[BUFFER_SIZE];
    if (strcmp(request_type, "GET") == 0) {
        printf("in the GET request\n");
        snprintf(request, sizeof(request), "GET /%s\r\n\r\n", file_name);
    } else if (strcmp(request_type, "POST") == 0) {
        printf("in the POST request\n");
        snprintf(request, sizeof(request), "POST /%s\r\n\r\n", file_name);
    } else {
        fprintf(stderr, "Invalid request type. Please use 'GET' or 'POST'.\n");
        exit(EXIT_FAILURE);
    }

    // Send the request
    send_request(request, SERVER_IP, SERVER_PORT);
    printf("Request sent!\n");

    return 0;
}


void send_request(const char *request, const char *server_ip, int server_port) {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    inet_aton(server_ip, &server_addr.sin_addr);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("Error connecting to the server");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    send(client_socket, request, strlen(request), 0);

    char buffer[BUFFER_SIZE];
    ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        perror("Error receiving data");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    buffer[received] = '\0';
    printf("%s\n", buffer);

    close(client_socket);
}
