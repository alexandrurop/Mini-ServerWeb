#define _CRT_SECURE_NO_WARNINGS
#include "../include/StaticFileHandler.h"
#include "../include/Util.h"
#include "../include/HttpResponse.h"
#include "../include/HttpRequest.h"
#include <fstream>

using namespace std;
namespace fs = std::filesystem;

StaticFileHandler::StaticFileHandler(const fs::path& root)
    : root_(root), error404_(root / "errors" / "404.html") {
}

string StaticFileHandler::sanitize_target(const string& target) {
    string path = target;
    auto q = path.find('?');
    if (q != string::npos) path = path.substr(0, q);
    if (path.empty() || path == "/") path = "/index.html";
    if (path.find("..") != string::npos) return {};
    string out = path;
    for (auto& c : out) if (c == '/') c = '\\';
    return out;
}

void StaticFileHandler::handle(const HttpRequest& req, HttpResponse& resp) {
    if (req.method != "GET" && req.method != "HEAD") {
        resp.status = 405; resp.reason = "Method Not Allowed";
        resp.body = "405 Method Not Allowed\n";
        resp.set_header("Content-Type", "text/plain; charset=utf-8");
        resp.set_header("Content-Length", to_string(resp.body.size()));
        resp.set_header("Connection", "close");
        return;
    }

    string local = sanitize_target(req.target);
    if (local.empty()) {
        resp.status = 400; resp.reason = "Bad Request";
        resp.body = "400 Bad Request\n";
        resp.set_header("Content-Type", "text/plain; charset=utf-8");
        resp.set_header("Content-Length", to_string(resp.body.size()));
        resp.set_header("Connection", "close");
        return;
    }

    fs::path full = root_ / fs::path(local).relative_path();
    if (fs::exists(full) && fs::is_regular_file(full)) {
        auto sz = fs::file_size(full);
        resp.status = 200; resp.reason = "OK";
        resp.set_header("Content-Type", util::guess_mime(full));
        resp.set_header("Content-Length", to_string(sz));
        resp.set_header("Connection", "close");
        if (req.method == "GET") {
            ifstream f(full, ios::binary);
            string data; data.resize((size_t)sz);
            f.read(data.data(), (std::streamsize)sz);
            resp.body = move(data);
        }
        return;
    }

    // 404
    if (fs::exists(error404_) && fs::is_regular_file(error404_)) {
        auto sz = fs::file_size(error404_);
        resp.status = 404; resp.reason = "Not Found";
        resp.set_header("Content-Type", "text/html; charset=utf-8");
        resp.set_header("Content-Length", to_string(sz));
        resp.set_header("Connection", "close");
        ifstream f(error404_, ios::binary);
        string data; data.resize((size_t)sz);
        f.read(data.data(), (std::streamsize)sz);
        resp.body = move(data);
    }
    else {
        resp.status = 404; resp.reason = "Not Found";
        resp.body = "404 Not Found\n";
        resp.set_header("Content-Type", "text/plain; charset=utf-8");
        resp.set_header("Content-Length", to_string(resp.body.size()));
        resp.set_header("Connection", "close");
    }
}
