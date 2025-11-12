#define _CRT_SECURE_NO_WARNINGS
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma comment(lib, "Ws2_32.lib")

static int send_all(SOCKET s, const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int r = send(s, buf + sent, len - sent, 0);
        if (r <= 0) return r;
        sent += r;
    }
    return sent;
}

static void usage(const char* exe) {
    fprintf(stderr,
        "Usage:\n"
        "  %s GET  <server_ip> <port> <path>\n"
        "  %s POST <server_ip> <port> <path> <body_text>\n"
        "Exemple:\n"
        "  %s GET  192.168.1.23 8080 /\n"
        "  %s GET  192.168.1.23 8080 /api/ping\n"
        "  %s POST 192.168.1.23 8080 /api/echo \"hello-mini-insta\"\n",
        exe, exe, exe, exe, exe);
}

int main(int argc, char** argv) {
    if (argc < 5) {
        usage(argv[0]);
        return 1;
    }

    const char* method = argv[1];
    const char* ip = argv[2];
    int port = atoi(argv[3]);
    const char* path = argv[4];

    int is_post = (_stricmp(method, "POST") == 0);
    int is_get = (_stricmp(method, "GET") == 0);

    if (!is_get && !is_post) {
        fprintf(stderr, "Method must be GET or POST\n");
        return 1;
    }

    const char* body = "";
    int body_len = 0;
    if (is_post) {
        if (argc < 6) {
            fprintf(stderr, "POST requires a body_text argument\n");
            return 1;
        }
        body = argv[5];
        body_len = (int)strlen(body);
    }

    // Winsock init
    WSADATA wsa;
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET) {
        fprintf(stderr, "socket failed\n");
        WSACleanup();
        return 1;
    }

    struct sockaddr_in addr;
    ZeroMemory(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &addr.sin_addr) != 1) {
        fprintf(stderr, "Invalid IP address\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (connect(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "connect failed (%d)\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    // Construim request-ul
    char req[4096];
    int n = 0;

    if (is_get) {
        n = snprintf(req, sizeof(req),
            "GET %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "User-Agent: mini_client_c/1.0\r\n"
            "Connection: close\r\n"
            "\r\n",
            path, ip, port);
    }
    else { // POST
        n = snprintf(req, sizeof(req),
            "POST %s HTTP/1.1\r\n"
            "Host: %s:%d\r\n"
            "User-Agent: mini_client_c/1.0\r\n"
            "Content-Type: text/plain; charset=utf-8\r\n"
            "Content-Length: %d\r\n"
            "Connection: close\r\n"
            "\r\n",
            path, ip, port, body_len);
    }

    if (n <= 0 || n >= (int)sizeof(req)) {
        fprintf(stderr, "Request too large or formatting error\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (send_all(s, req, n) <= 0) {
        fprintf(stderr, "send headers failed\n");
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (is_post && body_len > 0) {
        if (send_all(s, body, body_len) <= 0) {
            fprintf(stderr, "send body failed\n");
            closesocket(s);
            WSACleanup();
            return 1;
        }
    }

    // Citim r?spunsul ?i îl afi??m pe stdout
    char buf[4096];
    int r;
    while ((r = recv(s, buf, sizeof(buf), 0)) > 0) {
        fwrite(buf, 1, r, stdout);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
