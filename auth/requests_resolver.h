#include "auth_verbs.h"
#include "jsmn.h"

typedef struct _auth_request {
    auth_verb action;
    char *session_token;
    char *username;
    char *password;
} auth_request;

typedef struct _requests_resolver {
    char *(*callbacks[AUTH_VERBS_COUNT])(auth_request *);
} requests_resolver;

void requests_set_callback(requests_resolver *resolver, auth_verb action, char *(*callback)(auth_request *));

char *requests_resolve(requests_resolver *resolver, char *request);

