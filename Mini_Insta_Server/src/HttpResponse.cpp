#define _CRT_SECURE_NO_WARNINGS
#include "../include/HttpResponse.h"
using namespace std;

void HttpResponse::set_header(const string& k, const string& v) { headers[k] = v; }

string HttpResponse::serialize_headers_only() const {
    string s;
    s += "HTTP/1.1 " + to_string(status) + " " + reason + "\r\n";
    for (auto& kv : headers) {
        s += kv.first; s += ": "; s += kv.second; s += "\r\n";
    }
    s += "\r\n";
    return s;
}

string HttpResponse::serialize() const {
    string s = serialize_headers_only();
    s += body;
    return s;
}
