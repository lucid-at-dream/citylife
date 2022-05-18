#pragma once

enum _auth_verb
{
    AUTH_NEW,
    AUTH_DELETE,
    AUTH_CHANGE_PASSWORD,
    AUTH_AUTH,
    AUTH_INVALID_REQUEST,
    AUTH_VERBS_COUNT
};

typedef enum _auth_verb auth_verb;

auth_verb verb_translate_from_string(char *verb);