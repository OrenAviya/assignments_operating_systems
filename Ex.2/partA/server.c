//
// Created by aviyaob on 2/8/24.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/file.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>
#include <libgen.h>

#define BUFFER_SIZE 1024

void handle_request(int client_socket, const char *root_directory);

void handle_get_request(int client_socket, const char *remote_path, const char *root_directory);

void handle_post_request(int client_socket, const char *remote_path, const char *root_directory, const char *file_data);

void send_response(int client_socket, const char *response);

void Base64Encode(const char* message, char** buffer);

void Base64Decode(const char* input, char** output);


int main(int argc, char *argv[]) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

   int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(8080);

    if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1) {
        perror("Error binding socket");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 5) == -1) {
        perror("Error listening");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port 8080...\n");

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_len);
        if (client_socket == -1) {
            perror("Error accepting connection");
            continue;
        }

        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        pid_t child_pid = fork();
        if (child_pid == -1) {
            perror("Error forking process");
            close(client_socket);
            continue;
        } else if (child_pid == 0) {  // Child process
            printf("In the child process\n");
            close(server_socket);
            handle_request(client_socket, argv[1]);
            exit(EXIT_SUCCESS);
        } else {  // Parent process
            close(client_socket);
        }
    }

    close(server_socket);
    return 0;
}

void handle_request(int client_socket, const char *root_directory) {
    printf("Handle request...\n");
    char buffer[BUFFER_SIZE];

    ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        perror("Error receiving data");
        send_response(client_socket, "500 INTERNAL ERROR");
        close(client_socket);
        return;
    }

    buffer[received] = '\0';

    if (strstr(buffer, "GET") == buffer) {
        printf("In the GET request: \n");
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "GET %s", remote_path);
        handle_get_request(client_socket, remote_path, root_directory);
    } else if (strstr(buffer, "POST") == buffer) {
        printf("In the POST request: \n");
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "POST %s", remote_path);
        printf("Remote path: %s\n", remote_path);
        // Extract file data from the POST request
        char *file_data_start = strstr(buffer, "\r\n\r\n");
        // Check if file_data_start is not NULL and not empty
        if (file_data_start != NULL && *file_data_start != '\0') {
            printf("File data start: %s\n", file_data_start);
            handle_post_request(client_socket, remote_path, root_directory, file_data_start);
        } else {
            printf("No file data found in POST request\n");
            send_response(client_socket, "400 BAD REQUEST");
            close(client_socket);
            return;
        }

    } else {
        send_response(client_socket, "500 INTERNAL ERROR");
        close(client_socket);
        return;
    }

    close(client_socket);
}

void handle_get_request(int client_socket, const char *remote_path, const char *root_directory) {
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", root_directory, remote_path);

    FILE *file = fopen(full_path, "rb");

    if (file != NULL) {
        flock(fileno(file), LOCK_EX);  // Acquire file lock

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *contents = (char *) malloc(file_size + 1); // Extra byte for null terminator
        fread(contents, 1, file_size, file);
        contents[file_size] = '\0'; // Null terminate the string

        flock(fileno(file), LOCK_UN);  // Release file lock
        fclose(file);

        // Decode the content
        char *decoded_content;
        Base64Decode(contents, &decoded_content);

        // Send both the encoded and decoded content back to the client
        send_response(client_socket, "200 OK\n");
        // Sending decoded content
        send(client_socket, decoded_content, strlen(decoded_content), 0);

        free(contents);
        free(decoded_content);
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
    }
}




void handle_post_request(int client_socket, const char *remote_path, const char *root_directory, const char *file_data) {
    printf("Handle post request...\n");

    // Check if file_data is empty
    if (file_data == NULL || strlen(file_data) == 0) {
        send_response(client_socket, "400 BAD REQUEST");
        return;
    }

    printf("remote_path:%s\n", remote_path);
    char *file_name = basename((char *)remote_path);
    printf("File name: %s \n", file_name);

    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s/new_%s", root_directory, file_name);
    printf("Saving file to: %s\n", full_path);

    FILE *file = fopen(full_path, "wb"); // corrected file path
    if (file != NULL) {
        flock(fileno(file), LOCK_EX);  // Acquire file lock

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *contents = (char *) malloc(file_size + 1); // Extra byte for null terminator
        fread(contents, 1, file_size, file);
        contents[file_size] = '\0'; // Null terminate the string

        flock(fileno(file), LOCK_UN);  // Release file lock
        fclose(file);

        // Decode the content
        char *encoded_content;
        Base64Encode(contents, &encoded_content);

        // Send both the encoded and decoded content back to the client
        send_response(client_socket, "200 OK\n");
        // Sending decoded content
        send(client_socket, encoded_content, strlen(encoded_content), 0);

        free(contents);
        free(encoded_content);
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
    }
}

void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}
