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
#include "../base64.h"
#include <dirent.h>
#include "server.h"

#define BUFFER_SIZE 2048


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

        printf("Accepted connection from %s: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

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
        send_response(client_socket, "500 INTERNAL ERROR\n");
        close(client_socket);
        return;
    }

    buffer[received] = '\0';

    char request_type[BUFFER_SIZE];
    char filename[BUFFER_SIZE];

    // Parse the buffer to extract request type and filename
    if (sscanf(buffer, "%s %s", request_type, filename) != 2) {
        send_response(client_socket, "400 BAD REQUEST\n");
        close(client_socket);
        return;
    }

    printf("Request Type: %s\n", request_type);
    printf("Filename: %s\n", filename);

    if (strcmp(request_type, "GET") == 0) {
        // Handle GET request
        printf("Handle GET request\n");
        handle_get_request(client_socket, root_directory, filename);

    } else if (strcmp(request_type, "POST") == 0) {
        // Handle POST request
        printf("Handle POST request\n");

        // Find the start of the content
        char *content_start = strstr(buffer, "\n");
        if (content_start == NULL) {
            send_response(client_socket, "400 BAD REQUEST\n");
            close(client_socket);
            return;
        }
        content_start += 1; // move past "\n"

        // Extract content length
        int content_length = received - (content_start - buffer);

        // Allocate memory for content
        char *file_content = malloc(content_length + 1);
        if (file_content == NULL) {
            send_response(client_socket, "500 INTERNAL ERROR\n");
            close(client_socket);
            return;
        }

        // Copy content from buffer
        strncpy(file_content, content_start, content_length);
        file_content[content_length] = '\0';

        printf("File Content: %s\n", file_content);

        // Handle post request with content
        handle_post_request(client_socket, filename, root_directory, file_content, content_length);

        free(file_content);
    } else {
        send_response(client_socket, "500 INTERNAL ERROR\n");
        close(client_socket);
        return;
    }

    close(client_socket);
}


int is_image(const char *path) {
    const char *extensions[] = {".jpg", ".jpeg", ".png", ".gif", ".bmp"};
    for (size_t i = 0; i < sizeof(extensions) / sizeof(extensions[0]); i++) {
        if (strstr(path, extensions[i]) != NULL) {
            return 1;
        }
    }
    return 0;
}

// ********************************* Handle POST request *********************************  //

// TODO
void
handle_post_request(int client_socket, const char *remote_path, const char *root_directory, const char *file_content,
                    size_t content_length) {
    if (is_image(remote_path)) {
        //TODO
        printf("handle image\n");
//        handle_image_post_request(client_socket, remote_path, root_directory);
    } else {
        if (strstr(remote_path, ".list") != NULL) {
            handle_list_post_request(client_socket, remote_path, root_directory);
        } else {
            handle_file_post_request(client_socket, remote_path, root_directory, file_content, content_length);
        }
    }
}


// TODO
//void handle_image_post_request(client_socket, remote_path, root_directory) {
//
//}


void handle_file_post_request(int client_socket, const char *remote_path, const char *root_directory,
                              const char *file_content, size_t content_length) {
    printf("remote: %s\n", remote_path);
    // Set the name of the new file to "encoded_file"
    char *new_name;
    char *str_copy = strdup(remote_path);
    new_name = strtok(str_copy, ".");
    char new_file_path[BUFFER_SIZE];
    snprintf(new_file_path, BUFFER_SIZE, "%s/encoded_%s.txt", root_directory, new_name);

    // Open a new file with the generated file name
    FILE *new_file = fopen(new_file_path, "wb");
    if (new_file == NULL) {
        perror("Error opening file for writing");
        send_response(client_socket, "500 INTERNAL SERVER ERROR\n");
        return;
    }

    // Acquire an exclusive lock on the new file
    if (flock(fileno(new_file), LOCK_EX) == -1) {
        perror("Error acquiring lock on file");
        send_response(client_socket, "500 INTERNAL SERVER ERROR\n");
        fclose(new_file);
        return;
    }
    printf("file_content: %s\n", file_content);

    char *encoded_content;
    Base64Encode(file_content, &encoded_content);
    // Write the content of the file into the newly created file
    fwrite(encoded_content, 1, content_length, new_file);

    // Release the lock and close the file
    if (flock(fileno(new_file), LOCK_UN) == -1) {
        perror("Error releasing lock on file");
        send_response(client_socket, "500 INTERNAL SERVER ERROR\n");
        fclose(new_file);
        return;
    }

    // Close the file
    fclose(new_file);

    // Send a response to the client indicating success
    send_response(client_socket, "200 OK\n");
}


void handle_list_post_request(int client_socket, const char *remote_path, const char *root_directory) {
    // Open the list file
    printf("remote_path: %s\n", remote_path);
    FILE *list_file = fopen(remote_path, "r");
    if (list_file == NULL) {
        perror("Error opening list file");
        return;
    }
    printf("The list file opened %p\n", list_file);

    char line[256]; // Assuming a maximum line length of 255 characters

    while (fgets(line, sizeof(line), list_file) != NULL) {
        // Remove the newline character, if present
        line[strcspn(line, "\n")] = '\0';
        // Now line contains the file name
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "clientFiles/%s", line);
        printf("The file_path: %s\n", file_path);
        // Open the file to read its content
        FILE *file = fopen(file_path, "r");
        if (file == NULL) {
            perror("Error opening file");
            continue; // Move to the next file
        }
        printf("The file %p opened\n", file);

        // Determine the file size
        fseek(file, 0, SEEK_END);
        size_t file_size = ftell(file);
        rewind(file);

        // Read the file content
        char *file_content = (char *)malloc(file_size + 1);
        if (file_content == NULL) {
            perror("Memory allocation error");
            fclose(file);
            continue; // Move to the next file
        }
        fread(file_content, 1, file_size, file);
        file_content[file_size] = '\0'; // Null-terminate the content

        fclose(file);

        // Call handle_file_post_request with appropriate parameters
        handle_file_post_request(client_socket, line, root_directory, file_content, file_size);

        // Free dynamically allocated memory for file_content
        free(file_content);
    }
    printf("close the list file\n");

    fclose(list_file);
}


// ********************************* Handle GET request *********************************  //
void handle_get_request(int client_socket, const char *root_directory, const char *remote_path) {

    if (is_image(remote_path)) {
        //TODO
        printf("handle image\n");
//        handle_image_get_request(client_socket, remote_path, root_directory);
    } else {
        if (strstr(remote_path, ".list") != NULL) {
            handle_list_get_request(client_socket, root_directory, remote_path);
        } else {
            handle_file_get_request(client_socket, root_directory, remote_path);
        }

    }
}


void handle_file_get_request(int client_socket, const char *root_directory, const char *remote_path) {
    // Construct the full path to the remote file
    char full_path[256];
    snprintf(full_path, sizeof(full_path), "%s/%s", root_directory, remote_path);

    // Open the remote file for reading
    FILE *remote_file = fopen(full_path, "rb");
    if (remote_file == NULL) {
        perror("Error opening remote file");
        exit(EXIT_FAILURE);
    }

    // Determine the size of the remote file
    fseek(remote_file, 0, SEEK_END);
    long file_size = ftell(remote_file);
    rewind(remote_file);

    // Allocate memory to store the file content
    char *file_content = (char *) malloc(file_size + 1);
    if (file_content == NULL) {
        perror("Error allocating memory");
        fclose(remote_file);
        exit(EXIT_FAILURE);
    }

    // Read the content of the remote file
    size_t bytes_read = fread(file_content, 1, file_size, remote_file);
    if ((size_t) bytes_read != (size_t) file_size) {
        perror("Error reading remote file");
        fclose(remote_file);
        free(file_content);
        exit(EXIT_FAILURE);
    }

    // Close the remote file
    fclose(remote_file);

    // Null-terminate the file content
    file_content[file_size] = '\0';

    // Decode the file content if necessary
    char *decoded_content;
    Base64Decode(file_content, &decoded_content);

    // Construct the path for the new file
    char new_filename[256];
    snprintf(new_filename, sizeof(new_filename), "clientFiles/%s", remote_path);

    // Open the new file for writing
    FILE *new_file = fopen(new_filename, "wb");
    if (new_file == NULL) {
        perror("Error opening new file");
        free(decoded_content);
        free(file_content);
        exit(EXIT_FAILURE);
    }

    // Write the decoded content to the new file
    fwrite(decoded_content, sizeof(char), strlen(decoded_content), new_file);

    // Close the new file
    fclose(new_file);

    // Free the allocated memory
    free(decoded_content);
    free(file_content);

    send_response(client_socket, "200 OK\n");
    printf("File saved successfully to: %s\n", new_filename);
}

void handle_list_get_request(int client_socket, const char *root_directory, const char *remote_path) {
    // Open the list file
    printf("remote_path: %s\n", remote_path);
    FILE *list_file = fopen(remote_path, "r");
    if (list_file == NULL) {
        perror("Error opening list file");
        return;
    }
    printf("The list file opened %p\n", list_file);

    char line[256]; // Assuming a maximum line length of 255 characters

    while (fgets(line, sizeof(line), list_file) != NULL) {
        // Remove the newline character, if present
        line[strcspn(line, "\n")] = '\0';
        // Now line contains the file name
        char file_path[512];
        snprintf(file_path, sizeof(file_path), "%s", line);
        printf("The file_path: %s\n", file_path);

        // Call handle_file_post_request with appropriate parameters
        handle_file_get_request(client_socket, root_directory, line);

    }
    printf("close the list file\n");

    fclose(list_file);
}
void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}
