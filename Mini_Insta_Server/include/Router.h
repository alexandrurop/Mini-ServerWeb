#pragma once
#include "../include/HttpRequest.h"
#include "../include/HttpResponse.h"

class Router {
public:
    // Basic demo routes:
    //  GET  /api/ping  -> {"status":"ok"}
    //  POST /api/echo  -> echo body
    void handle(const HttpRequest& req, HttpResponse& resp);
};
