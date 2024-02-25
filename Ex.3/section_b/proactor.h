//
// Created by aviyaob on 2/25/24.
//

// section_b/proactor.h
#ifndef PROACTOR_H

#include <stddef.h>

typedef void (*ProactorHandler)(int socket);

void proactor_init();
void proactor_cleanup();
void proactor_add_socket(int socket, ProactorHandler handler);

#endif // PROACTOR_H
