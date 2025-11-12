#pragma once
#include <filesystem>
#include <string>
#include "../include/HttpRequest.h"
#include "../include/HttpResponse.h"

class StaticFileHandler {
    std::filesystem::path root_;
    std::filesystem::path error404_;
    static std::string sanitize_target(const std::string& target);
public:
    explicit StaticFileHandler(const std::filesystem::path& root);
    void handle(const HttpRequest& req, HttpResponse& resp);
};
