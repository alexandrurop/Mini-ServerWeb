#pragma once
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include <cstdint>
#include "../include/ThreadPool.h"
#include "../include/StaticFileHandler.h"
#include "../include/Router.h"

class HttpServer {
    uint16_t port_;
    SOCKET sfd_{ INVALID_SOCKET };
    ThreadPool pool_;
    StaticFileHandler static_handler_;
    Router router_;
    std::atomic<bool> running_{ false };

    static void winsock_init();
    static bool recv_request(SOCKET cs, HttpRequest& req);
    static void send_response(SOCKET cs, const HttpResponse& resp);
    void handle_client(SOCKET cs);

public:
    explicit HttpServer(uint16_t port, size_t workers = 4);
    void start();
    void stop();
};
