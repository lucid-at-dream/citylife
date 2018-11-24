#include "session_manager.h"

enum {
  SUCCESS,
  ERROR
} status;


typedef struct _result {
  int result;
  char *message;
} result;


result authenticate(char *user, char *token);

result add_user(char *user, char *password);

result del_user(char *user, session_token *token);

result change_password(char *user, session_token *token, char *new_password);

