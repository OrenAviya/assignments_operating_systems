//
// Created by aviyaob on 2/25/24.
//

// section_b/proactor.c
#include "proactor.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Structure to represent each socket and its corresponding handler
struct SocketHandler {
    int socket; // the socket file descriptor
    ProactorHandler handler; //  a function pointer to a ProactorHandler function
    struct SocketHandler* next; //a pointer to the next SocketHandler struct in a linked list.


};

// Global variables for managing the linked list of sockets and handlers
static struct SocketHandler* socket_handlers = NULL;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Worker thread function
void* worker_thread(void* arg) {
    int socket = *(int*)arg;

    // Find the handler for the given socket
    ProactorHandler handler = NULL;
    pthread_mutex_lock(&mutex);
    struct SocketHandler* current = socket_handlers;
    while (current != NULL) {
        if (current->socket == socket) {
            handler = current->handler;
            break;
        }
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);

    if (handler != NULL) {
        // Execute the handler for the socket
        handler(socket);
    }

    // Cleanup and exit the thread
    pthread_mutex_lock(&mutex);
    // Remove the socket and handler from the linked list
    struct SocketHandler* prev = NULL;
    current = socket_handlers;
    while (current != NULL) {
        if (current->socket == socket) {
            if (prev != NULL) {
                prev->next = current->next;
            } else {
                socket_handlers = current->next;
            }
            free(current);
            break;
        }
        prev = current;
        current = current->next;
    }
    pthread_mutex_unlock(&mutex);

    // Close the socket
    close(socket);

    return NULL;
}

void proactor_init() {
    // Initialize the proactor (if needed)
    pthread_mutex_init(&mutex, NULL);
}

void proactor_cleanup() {
    // Cleanup the proactor (if needed)
    pthread_mutex_destroy(&mutex);
}

void proactor_add_socket(int socket, ProactorHandler handler) {
    struct SocketHandler* new_socket_handler = (struct SocketHandler*)malloc(sizeof(struct SocketHandler));
    new_socket_handler->socket = socket;
    new_socket_handler->handler = handler;
    new_socket_handler->next = NULL;

    pthread_mutex_lock(&mutex);
    new_socket_handler->next = socket_handlers;
    socket_handlers = new_socket_handler;
    pthread_mutex_unlock(&mutex);

    pthread_t thread;
    if (pthread_create(&thread, NULL, worker_thread, &new_socket_handler->socket) != 0) {
        perror("pthread_create");
        pthread_mutex_lock(&mutex);
        free(new_socket_handler);
        pthread_mutex_unlock(&mutex);
        close(socket);
    } else {
        pthread_detach(thread);
    }
}