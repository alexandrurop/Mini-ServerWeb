#pragma once
#define _CRT_SECURE_NO_WARNINGS
#include <string>
#include <filesystem>
#include <winsock2.h>

namespace util {
    int send_all(SOCKET s, const char* buf, int len);
    std::string to_lower(std::string s);
    const char* guess_mime(const std::filesystem::path& p);
}
