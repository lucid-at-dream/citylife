#include "session_manager.h"

enum result_code { 
  SUCCESS, 
  ERROR 
};

typedef struct _result {
  enum result_code result;
  char *message;
} result;

result authenticate(char *user, char *token);

result add_user(char *user, char *password);

result del_user(char *user, session_token *token);

result change_password(char *user, session_token *token, char *new_password);

