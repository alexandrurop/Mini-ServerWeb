#pragma once
#include <winsock2.h>
#include <string>
#include <unordered_map>

class HttpRequest {
public:
    std::string method;
    std::string target;
    std::string version;
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    static bool parse_from_socket(SOCKET s, HttpRequest& out, int max_hdr = 8192);
};
