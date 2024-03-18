//
// Created by ofir on 2/29/24.
//

#ifndef ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H
#define ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H

void handle_request(int client_socket, const char *root_directory);

void handle_get_request(int client_socket, const char *remote_path, const char *root_directory);

void handle_post_request(int client_socket, const char *remote_path, const char *root_directory);

void send_response(int client_socket, const char *response);

#endif //ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H
