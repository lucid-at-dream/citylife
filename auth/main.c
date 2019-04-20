#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "server.h"
#include "requests_resolver.h"
#include "authenticator.h"
#include "worker_pool.h"

// Default service configuration
int port = 9999;

// CLI functions
void printUsage(FILE *stream);

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

int main(int argc, char **argv) {

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0) {
            if (i + 1 >= argc) {
                printUsage(stderr);
                exit(-1);
            }
            port = atoi(argv[++i]);
        }
        if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(stdout);
            exit(0);
        }
    }

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

void printUsage(FILE *stream) {
    fprintf(stream, "=== Usage -- Authentication microservice from Citylife ===\n");
    fprintf(stream, "auth.exe [-p|--port port] [-h|--help]\n");
    fprintf(stream, "\n");
    fprintf(stream, "-h|--help   Print this help message and exit\n");
    fprintf(stream, "-p|--port   The port in which to listen for requests\n");
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
