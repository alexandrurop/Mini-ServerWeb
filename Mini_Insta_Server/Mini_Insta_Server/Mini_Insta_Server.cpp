#include "../include/HttpServer.h"

int main() {
    HttpServer srv(8080, 4);
    srv.start();
    return 0;
}
