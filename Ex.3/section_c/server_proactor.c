//
// Created by aviyaob on 2/25/24.
//
//server_proactor.c
#include "proactor_lib/proactor.h"
#include <stdio.h>
#include <unistd.h>

void handle_connection(int socket) {
    // Handle the connection (e.g., read/write operations)
    printf("Handling connection on socket %d\n", socket);
}

int main() {
    proactor_init();

    // Simulate server behavior
    int server_socket = 123; // Replace with your actual server socket
    proactor_add_socket(server_socket, handle_connection);

    // Simulate another connection
    int client_socket = 456; // Replace with your actual client socket
    proactor_add_socket(client_socket, handle_connection);

    // Sleep to keep the program running for demonstration purposes
    sleep(5);

    proactor_cleanup();

    return 0;
}
