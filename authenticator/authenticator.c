#include "authenticator.h"
#include <stdlib.h>
#include <string.h>

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
  
  if (map_get(auth->auth_table, user) != NULL) {
    r.result = AUTH_ERROR;
    r.message = "The username is already registered.";
    return r;
  }

  map_set(auth->auth_table, user, password);
  
  r.result = AUTH_SUCCESS;
  r.message = "User has been added with success.";
  return r;
}

result authenticate(authenticator *auth, char *user, char *token) {
  result r;
  
  char *stored_password = map_get(auth->auth_table, user);

  int match = strcmp(stored_password, token);
  if (match == 0) {
    r.result = AUTH_SUCCESS;
  } else {
    r.result = AUTH_ERROR;
    r.message = "The wrong password has been provided for that username";
  }

  return r;
}

result change_password(authenticator *auth, char *user, char *token, char *new_password) {
  result r;

  if (authenticate(auth, user, token).result == AUTH_SUCCESS) {
    map_set(auth->auth_table, user, new_password);
    r.result = AUTH_SUCCESS;
  } else {
    r.result = AUTH_ERROR;
    r.message = "The provided password does not match the one associated to this user.";
  }

  return r;
}

result del_user(authenticator *auth, char *user, char *token) {
  result r;
  r.result = AUTH_ERROR;
  return r;
}
