#ifndef AUTH_SERVICE_H
#define AUTH_SERVICE_H

#include <windows.h>
#include <sqlext.h>

typedef struct {
    int authenticated;
    char user_handle[50];
} AuthResult;

AuthResult verify_login(const char* username, const char* password);

#endif