#include "server.h"
#include "requests_resolver.h"
#include "authenticator.h"

char *user_new(auth_request *request);
char *user_del(auth_request *request);
char *user_chpwd(auth_request *request);
char *user_auth(auth_request *request);

char *response_template = "{\n"
        "  \"result\": \"%s\",\n"
        "  \"message\": \"%s\"\n"
        "}";

int port = 9999;

authenticator *auth;

int main() {

    // Create a new authenticator
    auth = authenticator_new();

    // Setup request parsing
    requests_set_callback(AUTH_NEW, user_new;
    requests_set_callback(AUTH_DELETE, user_del);
    requests_set_callback(AUTH_CHANGE_PASSWORD, user_chpwd);
    requests_set_callback(AUTH_AUTH, user_auth);

    // TODO: Create a pool of worker threads
    // thread_pool *t = thread_pool_new(4);
    // thread_pool_set_handler(t, requests_resolve);

    // TODO: Create a server and start taking in requests
    // server *s = server_new(port);
    // server_set_worker_pool(s, queue *q);

    authenticator_destroy(auth);
    return 0;
}

char *encode_result_as_json(result *r) {
    char *response = (char *)calloc(1024, sizeof(char));
    sprintf(response, response_template, r->result == AUTH_SUCCESS ? "success" : "error", r->message);
    return response;
}

char *user_new(auth_request *request) {
    result r = add_user(auth, request->username, request->password);
    return encode_result_as_json(r);
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
