//
// Created by ofir on 2/29/24.
//

#ifndef ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H
#define ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H

void handle_request(int client_socket, const char *root_directory);

void handle_get_request(int client_socket, const char *remote_path, const char *root_directory);
void handle_post_request(int client_socket, const char *remote_path, const char *root_directory);

void handle_file_list_request(int client_socket, const char *full_path, const char *root_directory, char* type_request);
void handle_regular_file_request(int client_socket, const char *full_path,const char *root_directory, char* type_request);

void process_file_for_get(int client_socket, const char *file_path);
void process_file_for_post(int client_socket, const char *file_path,const char* root_directory);

char *construct_file_path(const char *root_directory, const char *file_name);
void trim_newline(char *str);


void send_response(int client_socket, const char *response);

#endif //ASSIGNMENTS_OPERATING_SYSTEMS_SERVER_H
