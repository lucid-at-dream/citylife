#include "auth_verbs.h"

#include <string.h>

auth_verb verb_translate_from_string(char *verb)
{
        if (strcmp(verb, "new") == 0) {
                return AUTH_NEW;

        } else if (strcmp(verb, "delete") == 0) {
                return AUTH_DELETE;

        } else if (strcmp(verb, "change_password") == 0) {
                return AUTH_CHANGE_PASSWORD;

        } else if (strcmp(verb, "auth") == 0) {
                return AUTH_AUTH;

        } else {
                return AUTH_INVALID_REQUEST;
        }
}
