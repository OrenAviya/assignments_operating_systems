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

    if (strstr(buffer, "GET") == buffer) {
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "GET %s", remote_path);
        handle_get_request(client_socket, remote_path, root_directory);
    } else if (strstr(buffer, "POST") == buffer) {
        char remote_path[BUFFER_SIZE];
        sscanf(buffer, "POST %s", remote_path);
        printf("Remote path: %s\n", remote_path);
        handle_post_request(client_socket, remote_path, root_directory);
        }
    else {
        send_response(client_socket, "500 INTERNAL ERROR\n");
        close(client_socket);
        return;
    }

    close(client_socket);

    }



// ********************************* Handle GET request *********************************  //
void handle_get_request(int client_socket, const char *remote_path, const char *root_directory) {
    printf("Handle GET request...\n");
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", root_directory, remote_path);
    printf("Path: %s\n", full_path);

    if (strstr(remote_path, ".list") != NULL) {
        handle_file_list_request(client_socket, full_path, root_directory, "GET");
    } else {
        handle_regular_file_request(client_socket, full_path,root_directory, "GET");
    }
    printf("GET request handled! \n\n");
}

void handle_file_list_request(int client_socket, const char *full_path, const char *root_directory, char* type_request) {
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
            if (strcmp(type_request, "GET") == 0){
                process_file_for_get(client_socket, file_path);
            } else {
                process_file_for_post(client_socket, file_path, root_directory);
            }
            free(file_path);
        }
    }

    fclose(file_list);
}

void handle_regular_file_request(int client_socket, const char *full_path,const char* root_directory, char* type_request) {
    FILE *file = fopen(full_path, "rb");
    if (file != NULL) {
        if (strcmp(type_request, "GET")) {
            process_file_for_get(client_socket, full_path);
        } else {
            process_file_for_post(client_socket, full_path,root_directory);
        }
        fclose(file);
    } else {
        send_response(client_socket, "404 FILE NOT FOUND");
        printf("Message for client: 404 FILE NOT FOUND\n");
    }
}

void process_file_for_get(int client_socket, const char *file_path) {
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

// ********************************* Handle POST request *********************************  //

void handle_post_request(int client_socket, const char *remote_path, const char *root_directory) {
    printf("Handle POST request...\n");
    char full_path[BUFFER_SIZE];
    snprintf(full_path, sizeof(full_path), "%s%s", root_directory, remote_path);

    if (strstr(remote_path, ".list") != NULL) {
        handle_file_list_request(client_socket, full_path, root_directory, "POST");
    } else {
        handle_regular_file_request(client_socket, full_path,root_directory, "POST");
    }
    printf("POST request handled! \n\n");
}

void process_file_for_post(int client_socket, const char *file_path, const char* root_directory) {
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

            char *encoded_content;
            Base64Encode(contents, &encoded_content);


            send_response(client_socket, "200 OK\n");
            send(client_socket, encoded_content, strlen(encoded_content), 0);

            // Save the encoded content as a new file in the root directory
            char *file_name = basename((char *)file_path);
            char *new_file_name = malloc(strlen(root_directory) + strlen("/") + strlen(file_name) + strlen("_encoded") + 1);
            if (new_file_name != NULL) {
                sprintf(new_file_name, "%s/%s_encoded", root_directory, file_name);

                FILE *new_file = fopen(new_file_name, "wb");
                if (new_file != NULL) {
                    fwrite(encoded_content, 1, strlen(encoded_content), new_file);
                    fclose(new_file);
                    printf("Saved file '%s' in '%s' directory.\n", new_file_name, root_directory);
                } else {
                    printf("Error: Couldn't save file '%s' in '%s' directory.\n", new_file_name, root_directory);
                }
                free(new_file_name);
            } else {
                printf("Error: Couldn't allocate memory for new file name.\n");
            }

            free(encoded_content);
            free(contents);
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



void send_response(int client_socket, const char *response) {
    send(client_socket, response, strlen(response), 0);
}
