#pragma once

#include "session_manager.h"
#include "map.h"

typedef struct _authenticator {
  map *auth_table;
} authenticator;

enum result_code { 
  SUCCESS, 
  ERROR 
};

typedef struct _result {
  enum result_code result;
  char *message;
} result;

authenticator *authenticator_new();

result authenticate(char *user, char *token);

result add_user(authenticator *auth, char *user, char *password);

result del_user(char *user, session_token *token);

result change_password(char *user, session_token *token, char *new_password);

