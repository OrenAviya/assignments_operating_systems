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
#include "../partB/base64.h"
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
        send_response(client_socket, "500 INTERNAL ERROR");
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
        printf("Remote path: %s\n", remote_path);
        // Extract file data from the POST request
        char *file_data_start = strstr(buffer, "\r\n\r\n");
        // Check if file_data_start is not NULL and not empty
        if (file_data_start != NULL && *file_data_start != '\0') {
            printf("File data start: %s\n", file_data_start);
            handle_post_request(client_socket, remote_path, root_directory);
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

// ************************ GET request ************************ //
void handle_get_request(int client_socket, const char *remote_path, const char *root_directory) {
    printf("Handle GET request...\n");
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", root_directory, remote_path);
    printf("Path: %s\n", full_path);

    if (strstr(remote_path, ".list") != NULL) {
        handle_file_list_request(client_socket, full_path, root_directory);
    } else {
        handle_regular_file_request(client_socket, full_path);
    }
    printf("GET request handled! \n\n");
}

void handle_file_list_request(int client_socket, const char *full_path, const char *root_directory) {
    printf("Client requests a list of files.\n");
    FILE *file_list = fopen(full_path, "r");
    if (file_list == NULL) {
        send_response(client_socket, "404 FILE LIST NOT FOUND");
        printf("Message for client: 404 FILE LIST NOT FOUND\n");
        return;
    }

    char file_name[BUFFER_SIZE];
    while (fgets(file_name, BUFFER_SIZE, file_list) != NULL) {
        trim_newline(file_name);
        char *file_path = construct_file_path(root_directory, file_name);

        if (file_path != NULL) {
            process_file(client_socket, file_path);
            free(file_path);
        }
    }

    fclose(file_list);
}

void handle_regular_file_request(int client_socket, const char *full_path) {
    FILE *file = fopen(full_path, "rb");
    if (file != NULL) {
        process_file(client_socket, full_path);
        fclose(file);
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
        printf("Message for client: 404 FILE NOT FOUND\n");
    }
}

void process_file(int client_socket, const char *file_path) {
    FILE *file = fopen(file_path, "rb");
    if (file != NULL) {
        flock(fileno(file), LOCK_EX);

        fseek(file, 0, SEEK_END);
        long file_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        char *contents = malloc(file_size + 1);
        if (contents != NULL) {
            fread(contents, 1, file_size, file);
            contents[file_size] = '\0';

            flock(fileno(file), LOCK_UN);
            fclose(file);

            char *decoded_content;
            Base64Decode(contents, &decoded_content);

            send_response(client_socket, "200 OK\n");
            send(client_socket, decoded_content, strlen(decoded_content), 0);

            // Save the decoded content as a new file in "clientFiles" folder
            char *file_name = basename((char *)file_path); // casting away constness
            char *client_file_path = malloc(strlen("clientFiles/") + strlen(file_name) + 1);
            if (client_file_path != NULL) {
                snprintf(client_file_path, strlen("clientFiles/") + strlen(file_name) + 1, "clientFiles/%s", file_name);
                FILE *client_file = fopen(client_file_path, "wb");
                if (client_file != NULL) {
                    fwrite(decoded_content, 1, strlen(decoded_content), client_file);
                    fclose(client_file);
                    printf("Saved file '%s' in clientFiles folder.\n", file_name);
                } else {
                    printf("Error: Couldn't save file '%s' in clientFiles folder.\n", file_name);
                }
                free(client_file_path);
            } else {
                perror("Error allocating memory for client_file_path");
            }

            free(contents);
            free(decoded_content);
            printf("\n");
        } else {
            perror("Error allocating memory for file contents");
            send_response(client_socket, "500 INTERNAL SERVER ERROR");
            fclose(file);
        }
    } else {
        printf("File '%s' not found.\n", file_path);
    }
}



char *construct_file_path(const char *root_directory, const char *file_name) {
    char *file_path = malloc(strlen(root_directory) + strlen(file_name) + 2);
    if (file_path != NULL) {
        snprintf(file_path, strlen(root_directory) + strlen(file_name) + 2, "%s/%s", root_directory, file_name);
    } else {
        perror("Error allocating memory for file_path");
    }
    return file_path;
}

void trim_newline(char *str) {
    size_t length = strlen(str);
    if (str[length - 1] == '\n') {
        str[length - 1] = '\0';
    }
}


void handle_post_request(int client_socket, const char *remote_path, const char *root_directory) {
    printf("Handle post request...\n");

    // Check if the remote path ends with ".list"
    if (strstr(remote_path, ".list") != NULL) {
        printf("Received file list: %s\n", remote_path);

        // Open the file list to read the file names
        FILE *file_list = fopen(remote_path, "r");
        if (file_list == NULL) {
            send_response(client_socket, "404 FILE LIST NOT FOUND");
            printf("Message for client: 404 FILE LIST NOT FOUND\n");
            return;
        }

        char file_name[BUFFER_SIZE];
        while (fgets(file_name, BUFFER_SIZE, file_list) != NULL) {
            // Remove trailing newline character if present
            size_t length = strlen(file_name);
            if (file_name[length - 1] == '\n') {
                file_name[length - 1] = '\0';
            }

            // Construct the full path for the file
            char *full_path = malloc(strlen(root_directory) + strlen(file_name) + 2); // 2 for '/', '\0'
            if (full_path == NULL) {
                perror("Error allocating memory for full_path");
                send_response(client_socket, "500 INTERNAL SERVER ERROR");
                fclose(file_list);
                return;
            }
            snprintf(full_path, strlen(root_directory) + strlen(file_name) + 2, "%s/%s", root_directory, file_name);

            printf("Source file path: %s\n", full_path);

            // Check if the listed file is itself a .list file
            if (strstr(file_name, ".list") != NULL) {
                // Recursive call to handle a POST request for the listed .list file
                handle_post_request(client_socket, full_path, root_directory);
            } else {
                // Construct the POST request for the current listed file
                char *post_request = malloc(strlen("POST ") + strlen(file_name) + 1); // +1 for null terminator
                if (post_request == NULL) {
                    perror("Error allocating memory for post_request");
                    send_response(client_socket, "500 INTERNAL SERVER ERROR");
                    free(full_path);
                    fclose(file_list);
                    return;
                }
                snprintf(post_request, strlen("POST ") + strlen(file_name) + 1, "POST %s", file_name);
                printf("Sending POST request for file: %s\n", file_name);

                // Open the listed file to read its contents
                FILE *listed_file = fopen(full_path, "rb");
                if (listed_file != NULL) {
                    // Read the contents of the listed file
                    fseek(listed_file, 0, SEEK_END);
                    long file_size = ftell(listed_file);
                    fseek(listed_file, 0, SEEK_SET);

                    char *contents = malloc(file_size + 1); // Extra byte for null terminator
                    if (contents == NULL) {
                        perror("Error allocating memory for file contents");
                        send_response(client_socket, "500 INTERNAL SERVER ERROR");
                        fclose(listed_file);
                        free(post_request);
                        free(full_path);
                        fclose(file_list);
                        return;
                    }
                    fread(contents, 1, file_size, listed_file);
                    contents[file_size] = '\0'; // Null terminate the string
                    fclose(listed_file);

                    // Forward the content to the handle_request function to simulate a POST request
                    handle_request(client_socket, contents);

                    free(contents);
                } else {
                    printf("File '%s' not found.\n", file_name);
                }

                free(post_request);
            }

            free(full_path);
        }

        fclose(file_list);
        send_response(client_socket, "200 OK\n");
        printf("\n");
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
        printf("Message for client: 404 FILE NOT FOUND\n");
    }
}

void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}