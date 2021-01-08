#include "service.h"
#include "server/server.h"
#include "requests_resolver.h"
#include "authenticator.h"
#include "logger/logger.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global variables (composition)
authenticator *auth;
requests_resolver resolver;
socket_server *server;

// Functions to resolve raw requests
char *handle_request(char *request);

// Functions for handling authentication requests
char *user_new(auth_request *request);
char *user_del(auth_request *request);
char *user_chpwd(auth_request *request);
char *user_auth(auth_request *request);

void service_run(int port)
{
    // Create a new authenticator
    auth = authenticator_new();

    // Setup request parsing
    requests_set_callback(&resolver, AUTH_NEW, user_new);
    requests_set_callback(&resolver, AUTH_DELETE, user_del);
    requests_set_callback(&resolver, AUTH_CHANGE_PASSWORD, user_chpwd);
    requests_set_callback(&resolver, AUTH_AUTH, user_auth);

    // Create a server and start taking in requests
    server = server_new(port);
    server_start(server, handle_request);

    info("service.c: Service clean shut down");
}

void service_stop()
{
    server_stop(server);
}

void service_del()
{
    info("Deallocating service resources");
    authenticator_destroy(auth);
    free(server);
    auth = NULL;
    server = NULL;
}

char *handle_request(char *request)
{
    return requests_resolve(&resolver, request);
}

char *encode_result_as_json(result *r)
{
    char *response_template = "{\n"
                              "  \"result\": \"%s\",\n"
                              "  \"message\": \"%s\"\n"
                              "}";

    char *response = (char *)calloc(4096, sizeof(char));
    snprintf(response, 4096 * sizeof(char), response_template, r->result == AUTH_SUCCESS ? "success" : "error", r->message);
    return response;
}

char *copy_string(char *string)
{
    int s;
    s = strlen(string);
    char *buffer = (char *)calloc(s + 1, sizeof(char));
    snprintf(buffer, (s + 1) * sizeof(char), string);
    return buffer;
}

char *user_new(auth_request *request)
{
    char *username = copy_string(request->username);
    char *password = copy_string(request->password);

    result r = add_user(auth, username, password);
    return encode_result_as_json(&r);
}

char *user_del(auth_request *request)
{
    char *username = request->username;
    char *password = request->password;

    result r = del_user(auth, username, password);
    return encode_result_as_json(&r);
}

char *user_chpwd(auth_request *request)
{
    char *username = request->username;
    char *session_token = request->session_token;
    char *password = copy_string(request->password);

    result r = change_password(auth, username, session_token, password);
    return encode_result_as_json(&r);
}

char *user_auth(auth_request *request)
{
    char *username = request->username; // No need to copy the values since these won't be stored.
    char *password = request->password;

    result r = authenticate(auth, username, password);
    return encode_result_as_json(&r);
}
