#pragma once

#include "session_manager.h"
#include "map.h"

typedef struct _authenticator {
  map *auth_table;
} authenticator;

enum result_code { 
  AUTH_SUCCESS, 
  AUTH_ERROR 
};

typedef struct _result {
  enum result_code result;
  char *message;
} result;

authenticator *authenticator_new();
void authenticator_destroy(authenticator *auth);

result authenticate(authenticator *auth, char *user, char *token);

result add_user(authenticator *auth, char *user, char *password);

result del_user(authenticator *auth, char *user, session_token *token);

result change_password(authenticator *auth, char *user, session_token *token, char *new_password);

