#include "authenticator.h"

/* enum {
  SUCCESS,
  ERROR
} status;

typedef struct _result {
  int status;
  char *message;
} result;
*/

result authenticate(char *user, char *token) {
  result r;
  r.result = ERROR;
  return r;
}

result add_user(char *user, char *password) {
  result r;
  r.result = SUCCESS;
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

