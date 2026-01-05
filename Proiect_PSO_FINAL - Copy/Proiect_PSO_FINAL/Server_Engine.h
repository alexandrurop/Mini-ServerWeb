#ifndef SERVER_ENGINE_H
#define SERVER_ENGINE_H

#include <winsock2.h>
#include "Routes.h"
#include "Post_Service.h"

typedef struct {
    SOCKET client_socket;
    struct Route* route_tree;
} ClientContext;

void start_server_engine(SOCKET server_socket, struct Route* route_tree);

DWORD WINAPI handle_client(LPVOID lpParam);

DWORD WINAPI worker_thread(LPVOID lpParam);

char* replace_tag(const char* source, const char* tag, const char* replacement);

#endif