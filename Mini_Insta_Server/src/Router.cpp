#include "../include/Router.h"
#include "../include/Util.h"

void Router::handle(const HttpRequest& req, HttpResponse& resp) {
    // Demo rute foarte simple
    if (req.method == "GET" && req.target == "/api/ping") {
        resp.status = 200; resp.reason = "OK";
        resp.body = "{\"status\":\"ok\"}\n";
        resp.set_header("Content-Type", "application/json; charset=utf-8");
        resp.set_header("Content-Length", std::to_string(resp.body.size()));
        resp.set_header("Connection", "close");
        return;
    }
    if (req.method == "POST" && req.target == "/api/echo") {
        resp.status = 200; resp.reason = "OK";
        resp.body = req.body; // echo raw body (text)
        resp.set_header("Content-Type", "text/plain; charset=utf-8");
        resp.set_header("Content-Length", std::to_string(resp.body.size()));
        resp.set_header("Connection", "close");
        return;
    }
    // Dacă nu e o rută /api/ recunoscută, semnalăm 404 aici;
    // HttpServer va încerca static doar când path NU începe cu /api/
    resp.status = 404; resp.reason = "Not Found";
    resp.body = "404 Not Found (API)\n";
    resp.set_header("Content-Type", "text/plain; charset=utf-8");
    resp.set_header("Content-Length", std::to_string(resp.body.size()));
    resp.set_header("Connection", "close");
}
