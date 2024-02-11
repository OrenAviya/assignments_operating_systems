#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <openssl/bio.h>
#include <openssl/evp.h>

#define MAX_EVENTS 10
#define MAX_BUFFER_SIZE 1024


#include <stdio.h>
#include <stdarg.h>

#define LOG_LEVEL_INFO    0
#define LOG_LEVEL_WARNING 1
#define LOG_LEVEL_ERROR   2

int global_log_level = LOG_LEVEL_INFO; // Default log level


void set_log_level(int level) {
    global_log_level = level;
}
// Instead of using print - we will use log
void log_message(int level, const char *format, ...) {
    if (level < global_log_level) {
        return;
    }

    va_list args;
    va_start(args, format);

    if (level == LOG_LEVEL_INFO) {
        printf("[INFO]: ");
    } else if (level == LOG_LEVEL_WARNING) {
        printf("[WARNING]: ");
    } else if (level == LOG_LEVEL_ERROR) {
        printf("[ERROR]: ");
    } else {
        printf("[UNKNOWN]: ");
    }

    vprintf(format, args);
    printf("\n");

    va_end(args);
}

// Function to set a socket to non-blocking mode
int set_non_blocking(int sockfd) {
    int flags = fcntl(sockfd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl");
        return -1;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl");
        return -1;
    }
    return 0;
}

// Function to decode base64-encoded content
char* base64_decode(const char* input) {
    BIO *bio, *b64;
    int decoded_size = 0;
    char *buffer;

    // Create a base64 filter/sink
    b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);

    // Create a memory BIO to store the decoded output
    bio = BIO_new(BIO_s_mem());

    // Chain the BIOs: base64 filter -> memory BIO
    BIO_push(b64, bio);

    // Decode the base64 data
    BIO_write(b64, input, strlen(input));
    BIO_flush(b64);

    // Get the decoded data length
    decoded_size = BIO_get_mem_data(bio, &buffer);

    // Allocate memory for the decoded data
    char *decoded_data = (char*)malloc(decoded_size + 1);

    // Copy the decoded data to the buffer
    memcpy(decoded_data, buffer, decoded_size);
    decoded_data[decoded_size] = '\0';

    // Clean up
    BIO_free_all(b64);

    return decoded_data;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <server_address> <file_to_download>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Parse command line arguments
    char *server_address = argv[1];
    char *file_to_download = argv[2];
    log_message(LOG_LEVEL_INFO, "server_address: %p", server_address);
    log_message(LOG_LEVEL_INFO, "file_to_download: %p", file_to_download);


    // Resolve server address
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(server_address, "8080", &hints, &res) != 0) {
        perror("getaddrinfo");
        exit(EXIT_FAILURE);
    }
    log_message(LOG_LEVEL_INFO, "success resolve server address : %p", server_address);

    // Create epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    log_message(LOG_LEVEL_INFO, "success create epoll instance");

    // Iterate through each resolved address and connect
    struct addrinfo *p;
    int sockfd;
    for (p = res; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) {
            perror("socket");
            continue;
        }
        log_message(LOG_LEVEL_INFO, "created socket!");
        // Set socket to non-blocking mode
        if (set_non_blocking(sockfd) == -1) {
            exit(EXIT_FAILURE);
        }

        // Connect
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1 && errno != EINPROGRESS) {
            perror("connect");
            close(sockfd);
            continue;
        }
        log_message(LOG_LEVEL_INFO, "success to connect!!!");

        // Add socket to epoll
        struct epoll_event event;
        event.data.fd = sockfd;
        event.events = EPOLLOUT;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &event) == -1) {
            perror("epoll_ctl");
            exit(EXIT_FAILURE);
        }
        log_message(LOG_LEVEL_INFO, "success add socket to epoll");
    }

    // Prepare request
    char request[MAX_BUFFER_SIZE];
    snprintf(request, sizeof(request), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", file_to_download, server_address);
    log_message(LOG_LEVEL_INFO, "success prepare the request");

    // Main loop
    struct epoll_event events[MAX_EVENTS];
    while (1) {
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < num_events; i++) {
            if (events[i].events & EPOLLOUT) {
                // Socket is ready for writing
                if (send(events[i].data.fd, request, strlen(request), 0) == -1) {
                    perror("send");
                    close(events[i].data.fd);
                    continue;
                }
                log_message(LOG_LEVEL_INFO, "Socket is ready for writing");
                // Modify event to wait for read
                events[i].events = EPOLLIN;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]) == -1) {
                    perror("epoll_ctl");
                    exit(EXIT_FAILURE);
                }
                log_message(LOG_LEVEL_INFO, "Modify event to wait for read");
            } else if (events[i].events & EPOLLIN) {
                // Socket is ready for reading
                char response[MAX_BUFFER_SIZE];
                int num_bytes = recv(events[i].data.fd, response, sizeof(response), 0);
                if (num_bytes == -1) {
                    perror("recv");
                    close(events[i].data.fd);
                    log_message(LOG_LEVEL_INFO, "Socket is ready for reading");
                    continue;
                } else if (num_bytes == 0) {
                    // Connection closed by server
                    log_message(LOG_LEVEL_INFO, "Connection closed by server");
                    close(events[i].data.fd);
                    continue;
                } else {
                    // Process received data
                    char* decoded_response = base64_decode(response);
                    printf("Decoded response:\n%s\n", decoded_response);
                    free(decoded_response);
                }
            }
        }
    }

    // Clean up
    freeaddrinfo(res);
    close(epoll_fd);

    return 0;
}
