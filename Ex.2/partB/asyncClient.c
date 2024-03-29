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
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <POST/GET> <file_name/.list> \n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *request_type = argv[1];
    const char *file_name = argv[2];
    int server_port = 8080;
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
    if (strstr(file_name, ".list") != NULL) {
        char file_path[BUFFER_SIZE];
        snprintf(file_path, BUFFER_SIZE, "%s", file_name);
        printf("file_path: %s\n", file_path);
        FILE *file = fopen(file_path, "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // Read the file content
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        char file_content[file_size + 1];
        fread(file_content, 1, file_size, file);
        fclose(file);
        file_content[file_size] = '\0';

        printf("file_content: %s\n", file_content);
        printf("file_name: %s\n", file_name);
        snprintf(request, BUFFER_SIZE, "%s %s\n%s", request_type, file_name, file_content);
    } else {
        if (strcmp(request_type, "GET") == 0) {
            snprintf(request, BUFFER_SIZE, "%s %s", request_type, file_name);
        } else {

        char file_path[BUFFER_SIZE];
        snprintf(file_path, BUFFER_SIZE, "clientFiles/%s", file_name);
        printf("file_path: %s\n", file_path);
        FILE *file = fopen(file_path, "r");
        if (!file) {
            perror("Error opening file");
            exit(EXIT_FAILURE);
        }

        // Read the file content
        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        rewind(file);
        char file_content[file_size + 1];
        fread(file_content, 1, file_size, file);
        fclose(file);
        file_content[file_size] = '\0';

        printf("file_content: %s\n", file_content);
        printf("file_name: %s\n", file_name);
        snprintf(request, BUFFER_SIZE, "%s %s\n%s", request_type, file_name, file_content);
        }

    }

    // Send the request to the server
    send_request(client_socket, request);
    printf("%s request sent!\n", request_type);

    // Receive and print server response
    char response[BUFFER_SIZE];
    ssize_t bytes_received;

    // If request is for a list, wait for multiple responses
    if (strstr(file_name, ".list") != NULL) {
        while ((bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0)) > 0) {
            response[bytes_received] = '\0';
            printf("Server response: %s\n", response);
        }
        if (bytes_received == -1) {
            perror("Error receiving response");
        }
    } else { // Single response for other file requests
        bytes_received = recv(client_socket, response, BUFFER_SIZE - 1, 0);
        if (bytes_received == -1) {
            perror("Error receiving response");
        } else {
            response[bytes_received] = '\0';
            printf("Server response: %s\n", response);
        }
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