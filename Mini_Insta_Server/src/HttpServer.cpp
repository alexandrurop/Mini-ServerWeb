#define _CRT_SECURE_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN
#include "../include/HttpServer.h"
#include "../include/Util.h"
#include <cstdio>
#include <filesystem>
#include <fstream>

#pragma comment(lib, "Ws2_32.lib")

using namespace std;
namespace fs = std::filesystem;

static void close_socket_safe(SOCKET s) {
    if (s != INVALID_SOCKET) closesocket(s);
}

HttpServer::HttpServer(uint16_t port, size_t workers)
    : port_(port), pool_(workers ? workers : 4), static_handler_("./www") {
}

void HttpServer::winsock_init() {
    static std::atomic<bool> inited{ false };
    static std::mutex m;
    std::lock_guard<std::mutex> lk(m);
    if (!inited.load()) {
        WSADATA wsa;
        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
            fprintf(stderr, "WSAStartup failed\n");
            exit(1);
        }
        inited = true;
    }
}

bool HttpServer::recv_request(SOCKET cs, HttpRequest& req) {
    return HttpRequest::parse_from_socket(cs, req);
}

void HttpServer::send_response(SOCKET cs, const HttpResponse& resp) {
    std::string head = resp.serialize_headers_only();
    if (util::send_all(cs, head.c_str(), (int)head.size()) <= 0) return;
    if (!resp.body.empty()) {
        util::send_all(cs, resp.body.data(), (int)resp.body.size());
    }
}

void HttpServer::handle_client(SOCKET cs) {
    HttpRequest req;
    if (!recv_request(cs, req)) {
        HttpResponse r; r.status = 400; r.reason = "Bad Request";
        r.body = "400 Bad Request\n";
        r.set_header("Content-Type", "text/plain; charset=utf-8");
        r.set_header("Content-Length", std::to_string(r.body.size()));
        r.set_header("Connection", "close");
        send_response(cs, r);
        close_socket_safe(cs);
        return;
    }

    HttpResponse resp;

    // Router simplu: dacă path începe cu /api/, pasezi la Router; altfel servești static
    if (req.target.rfind("/api/", 0) == 0) {
        router_.handle(req, resp);
    }
    else {
        static_handler_.handle(req, resp);
    }

    send_response(cs, resp);
    close_socket_safe(cs);
}

void HttpServer::start() {
    winsock_init();

    // Creează www minim, ca să răspundă imediat
    try {
        fs::create_directories("www/errors");
        if (!fs::exists("www/index.html")) {
            std::ofstream f("www/index.html"); f << "<h1>Mini-Insta (basic)</h1>\n";
        }
        if (!fs::exists("www/errors/404.html")) {
            std::ofstream f("www/errors/404.html"); f << "<h1>404 Not Found</h1>\n";
        }
    }
    catch (...) {}

    sfd_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sfd_ == INVALID_SOCKET) { fprintf(stderr, "socket failed\n"); exit(1); }

    BOOL yes = 1;
    setsockopt(sfd_, SOL_SOCKET, SO_REUSEADDR, (const char*)&yes, sizeof(yes));

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    if (::bind(sfd_, (sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "bind failed (%d)\n", WSAGetLastError()); exit(1);
    }
    if (listen(sfd_, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "listen failed (%d)\n", WSAGetLastError()); exit(1);
    }

    running_ = true;
    printf("Server pe 0.0.0.0:%u\n", port_);

    while (running_) {
        sockaddr_in cli; int clen = sizeof(cli);
        SOCKET cs = accept(sfd_, (sockaddr*)&cli, &clen);
        if (cs == INVALID_SOCKET) {
            int e = WSAGetLastError();
            if (e == WSAEINTR) continue;
            fprintf(stderr, "accept failed (%d)\n", e);
            continue;
        }
        pool_.submit([this, cs] { handle_client(cs); });
    }
}

void HttpServer::stop() {
    running_ = false;
    if (sfd_ != INVALID_SOCKET) { closesocket(sfd_); sfd_ = INVALID_SOCKET; }
    WSACleanup();
}
