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

#define BUFFER_SIZE 1024

void handle_request(int client_socket, const char *root_directory);

void handle_get_request(int client_socket, const char *remote_path, const char *root_directory);

void handle_post_request(int client_socket, const char *remote_path, const char *root_directory);

void send_response(int client_socket, const char *response);

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
    char buffer[BUFFER_SIZE];

    ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        perror("Error receiving data");
        close(client_socket);
        return;
    }

    buffer[received] = '\0';

    if (strstr(buffer, "GET") == buffer) {
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "GET %s", remote_path);
        handle_get_request(client_socket, remote_path, root_directory);
    } else if (strstr(buffer, "POST") == buffer) {
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "POST %s", remote_path);
        handle_post_request(client_socket, remote_path, root_directory);
    } else {
        send_response(client_socket, "500 INTERNAL ERROR");
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

        char *contents = (char *) malloc(file_size);
        fread(contents, 1, file_size, file);

        flock(fileno(file), LOCK_UN);  // Release file lock
        fclose(file);

        // Encode contents using OpenSSL base64 encoding
        BIO *bio, *b64;
        BUF_MEM *bptr;

        b64 = BIO_new(BIO_f_base64());
        BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
        bio = BIO_new(BIO_s_mem());
        BIO_push(b64, bio);

        BIO_write(b64, contents, file_size);
        BIO_flush(b64);
        BIO_get_mem_ptr(b64, &bptr);

        char *encoded_contents = (char *) malloc(bptr->length + 1);
        memcpy(encoded_contents, bptr->data, bptr->length);
        encoded_contents[bptr->length] = '\0';

        BIO_free_all(b64);
        free(contents);

        char response[BUFFER_SIZE];
        snprintf(response, sizeof(response), "200 OK\n%s\n", encoded_contents);
        send_response(client_socket, response);

        free(encoded_contents);
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
    }
}

void handle_post_request(int client_socket, const char *remote_path, const char *root_directory) {
    char buffer[BUFFER_SIZE];
    ssize_t received = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (received == -1) {
        perror("Error receiving data");
        send_response(client_socket, "500 INTERNAL ERROR");
        return;
    }

    buffer[received] = '\0';

    char *contents = strstr(buffer, "\n") + 1;

    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", root_directory, remote_path);

    FILE *file = fopen(full_path, "wb");
    if (file != NULL) {
        flock(fileno(file), LOCK_EX);  // Acquire file lock

        // Decode contents using OpenSSL base64 decoding
        BIO *b64, *bio;
        b64 = BIO_new(BIO_f_base64());
        bio = BIO_new_mem_buf(contents, -1);
        bio = BIO_push(b64, bio);

        char decoded_contents[BUFFER_SIZE];
        int decoded_size = BIO_read(bio, decoded_contents, sizeof(decoded_contents));
        decoded_contents[decoded_size] = '\0';

        fwrite(decoded_contents, 1, decoded_size, file);

        BIO_free_all(bio);

        flock(fileno(file), LOCK_UN);  // Release file lock
        fclose(file);

        send_response(client_socket, "200 OK");
    } else {
        send_response(client_socket, "500 INTERNAL ERROR");
    }
}

void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}
