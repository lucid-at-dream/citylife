#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "requests_resolver.h"

/*Private functions*/
auth_request *parse_request(char *request);

/*Implementation*/
void requests_set_callback(requests_resolver *resolver, auth_verb action, char *(*callback)(auth_request *)) {
    resolver->callbacks[action] = callback;
}

void free_request(auth_request *r) {
    if (r->username)
        free(r->username);
    if (r->password)
        free(r->password);
    if (r->session_token)
        free(r->session_token);
    if (r)
        free(r);
}

char *requests_resolve(requests_resolver *resolver, char *request) {
    auth_request *r = parse_request(request);

    char *response;

    if (r != NULL) {
        response = resolver->callbacks[r->action](r);
        free_request(r);
    } else {
        response = "{msg: \"Bad request, you scumbag!\"}";
    }

    return response;
}

auth_request *parse_request(char *request) {
    // Initialize parser
    jsmn_parser parser;
    jsmn_init(&parser);

    // Parse the request json
    jsmntok_t tokens[256];
    int num_tokens = jsmn_parse(&parser, request, strlen(request), tokens, 256);

    // Initialize the return value of this function
    auth_request *r = (auth_request *)calloc(1, sizeof(auth_request));

    for (int i = 0; i < num_tokens; i++) {
        jsmntok_t tok = tokens[i];

        // Let's go for the keys of the json object.
        if (tok.type == JSMN_STRING && tok.size == 1) {
            // Extract the key
            int key_len = tok.end - tok.start;
            char key[key_len + 1];
            for (int j = 0; j < key_len; j++) {
                key[j] = request[tok.start + j];
            }
            key[key_len] = '\0';

            // Extract the value
            tok = tokens[++i];
            int value_len = tok.end - tok.start;
            char *value = (char *)calloc(value_len + 1, sizeof(char));
            for (int j = 0; j < value_len; j++) {
                value[j] = request[tok.start + j];
            }
            value[value_len] = '\0';

            if (strcmp(key, "action") == 0) {
                r->action = verb_translate_from_string(value);
                free(value);
            } else if (strcmp(key, "user") == 0) {
                r->username = value;
            } else if (strcmp(key, "pass") == 0) {
                r->password = value;
            } else if (strcmp(key, "sess") == 0) {
                r->session_token = value;
            } else {
                return NULL;
            }
        }
    }
    return r;
}
