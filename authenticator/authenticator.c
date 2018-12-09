#include "authenticator.h"
#include <stdlib.h>

authenticator *authenticator_new() {
  authenticator *auth = (authenticator *)calloc(1, sizeof(authenticator));
  auth->auth_table = map_new(16);
  return auth;  
}

void authenticator_destroy(authenticator *auth) {
  map_destroy(auth->auth_table);
  free(auth);
}

result add_user(authenticator *auth, char *user, char *password) {
  result r;
  map_set(auth->auth_table, user, password);
  r.result = SUCCESS;
  return r;
}

result authenticate(char *user, char *token) {
  result r;
  r.result = ERROR;
  return r;
}

result del_user(char *user, session_token *token) {
  result r;
  r.result = ERROR;
  return r;
}

result change_password(char *user, session_token *token, char *new_password) {
  result r;
  r.result = ERROR;
  return r;
}

