#define _CRT_SECURE_NO_WARNINGS
#include "../include/HttpRequest.h"
#include <cstdio>
#include <string>

using namespace std;

bool HttpRequest::parse_from_socket(SOCKET s, HttpRequest& out, int max_hdr) {
    string buf; buf.reserve(max_hdr);
    char tmp[2048];

    while ((int)buf.size() < max_hdr) {
        int r = recv(s, tmp, (int)sizeof(tmp), 0);
        if (r == 0) break;
        if (r < 0) return false;
        buf.append(tmp, tmp + r);

        size_t pos = buf.find("\r\n\r\n");
        if (pos != string::npos) {
            string head = buf.substr(0, pos);
            string tail = buf.substr(pos + 4);

            // start-line
            size_t eol = head.find("\r\n");
            string start = (eol == string::npos ? head : head.substr(0, eol));
            {
                char m[16] = { 0 }, t[2048] = { 0 }, v[16] = { 0 };
                if (sscanf(start.c_str(), "%15s %2047s %15s", m, t, v) != 3) return false;
                out.method = m; out.target = t; out.version = v;
            }
            // headers
            size_t idx = (eol == string::npos ? head.size() : eol + 2);
            while (idx < head.size()) {
                size_t next = head.find("\r\n", idx);
                string line = head.substr(idx, (next == string::npos ? head.size() - idx : next - idx));
                idx = (next == string::npos ? head.size() : next + 2);
                size_t colon = line.find(':');
                if (colon != string::npos) {
                    string k = line.substr(0, colon);
                    string v = line.substr(colon + 1);
                    while (!v.empty() && (v.front() == ' ' || v.front() == '\t')) v.erase(v.begin());
                    for (auto& c : k) c = (char)tolower((unsigned char)c);
                    out.headers[k] = v;
                }
            }
            // body (Content-Length)
            auto it = out.headers.find("content-length");
            if (it != out.headers.end()) {
                int cl = atoi(it->second.c_str());
                if ((int)tail.size() >= cl) { out.body = tail.substr(0, cl); return true; }
                string bodybuf = tail;
                while ((int)bodybuf.size() < cl) {
                    int r2 = recv(s, tmp, (int)sizeof(tmp), 0);
                    if (r2 <= 0) return false;
                    bodybuf.append(tmp, tmp + r2);
                }
                out.body = bodybuf.substr(0, cl);
            }
            else {
                out.body.clear();
            }
            return true;
        }
    }
    return false;
}
