#include "auth_verbs.h"

typedef struct _auth_request {
    auth_verb action;
    char *session_token;
    char *username;
    char *password;
} auth_request;

void requests_set_callback(auth_verb action, char *(*callback)(auth_request *));

char *requests_resolve(char *request);

