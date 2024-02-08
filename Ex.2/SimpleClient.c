//
// Created by aviyaob on 2/8/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void send_request(const char *request, const char *server_ip, int server_port);

int main() {
    const char *server_ip = "127.0.0.1";
    int server_port = 8080;

    // Test GET request
    const char *get_request = "GET /test_file.txt\r\n\r\n";
    send_request(get_request, server_ip, server_port);

    // Test POST request
    const char *contents = "SGVsbG8gd29ybGQh"; // Base64 encoded "Hello world!"
    char post_request[BUFFER_SIZE];
    snprintf(post_request, sizeof(post_request), "POST /uploaded_file.txt\r\n%s\r\n\r\n", contents);
    send_request(post_request, server_ip, server_port);

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

