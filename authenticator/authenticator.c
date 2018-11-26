#include "authenticator.h"
#include "map.h"
#include <stddef.h>

/* enum {
  SUCCESS,
  ERROR
} status;

typedef struct _result {
  int status;
  char *message;
} result;
*/

map *auth_table;

result add_user(char *user, char *password) {
  if (auth_table == NULL) {
    auth_table = map_new(16);
  }
  result r;
  map_add(auth_table, user, password);
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

