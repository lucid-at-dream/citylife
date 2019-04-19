#include <stdlib.h>
#include <stdio.h>

#include "server.h"
#include "requests_resolver.h"
#include "authenticator.h"
#include "worker_pool.h"

// Default service configuration
int port = 9999;

// Functions to resolve raw requests
char *handle_request(char *request);
char *add_work_to_pool(char *request);

// Functions for handling authentication requests
char *user_new(auth_request *request);
char *user_del(auth_request *request);
char *user_chpwd(auth_request *request);
char *user_auth(auth_request *request);

// Global variables (composition)
authenticator *auth;
requests_resolver resolver;
worker_pool *pool;

int main() {

    // Create a new authenticator
    auth = authenticator_new();

    // Setup request parsing
    requests_set_callback(&resolver, AUTH_NEW, user_new);
    requests_set_callback(&resolver, AUTH_DELETE, user_del);
    requests_set_callback(&resolver, AUTH_CHANGE_PASSWORD, user_chpwd);
    requests_set_callback(&resolver, AUTH_AUTH, user_auth);

    // TODO: Use a thread pool to handle the requests
    // pool = pool_new(4, handle_request);
    // pool_start(pool);

    // Create a server and start taking in requests
    socket_server *server = server_new(port);
    server_start(server, handle_request);

    return 0;
}

char *handle_request(char *request) {
    return requests_resolve(&resolver, request);
}

char *encode_result_as_json(result *r) {
    char *response_template = "{\n"
        "  \"result\": \"%s\",\n"
        "  \"message\": \"%s\"\n"
        "}";

    char *response = (char *)calloc(1024, sizeof(char));
    sprintf(response, response_template, r->result == AUTH_SUCCESS ? "success" : "error", r->message);
    return response;
} 

char *user_new(auth_request *request) {
    result r = add_user(auth, request->username, request->password);
    return encode_result_as_json(&r);
}

char *user_del(auth_request *request) {
    result r = del_user(auth, request->username, request->password);
    return encode_result_as_json(&r);
}

char *user_chpwd(auth_request *request) {
    result r = change_password(auth, request->username, request->session_token, request->password);
    return encode_result_as_json(&r);
}

char *user_auth(auth_request *request) {
    result r = authenticate(auth, request->username, request->password);
    return encode_result_as_json(&r);
}
