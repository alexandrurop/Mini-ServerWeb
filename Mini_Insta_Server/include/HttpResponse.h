#pragma once
#include <string>
#include <unordered_map>

class HttpResponse {
public:
    int status = 200;
    std::string reason = "OK";
    std::unordered_map<std::string, std::string> headers;
    std::string body;

    void set_header(const std::string& k, const std::string& v);
    std::string serialize_headers_only() const;
    std::string serialize() const;
};
