#define _CRT_SECURE_NO_WARNINGS
#include "../include/Util.h"
#include <cctype>

using namespace std;
namespace fs = std::filesystem;

int util::send_all(SOCKET s, const char* buf, int len) {
    int sent = 0;
    while (sent < len) {
        int r = send(s, buf + sent, len - sent, 0);
        if (r <= 0) return r;
        sent += r;
    }
    return sent;
}

std::string util::to_lower(std::string s) {
    for (auto& c : s) c = (char)tolower((unsigned char)c);
    return s;
}

const char* util::guess_mime(const fs::path& p) {
    auto ext = util::to_lower(p.extension().string());
    if (ext == ".html" || ext == ".htm") return "text/html; charset=utf-8";
    if (ext == ".css")  return "text/css; charset=utf-8";
    if (ext == ".js")   return "application/javascript";
    if (ext == ".png")  return "image/png";
    if (ext == ".jpg" || ext == ".jpeg") return "image/jpeg";
    if (ext == ".gif")  return "image/gif";
    if (ext == ".svg")  return "image/svg+xml";
    return "application/octet-stream";
}
