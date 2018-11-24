#include <stdio.h>

#include "test.h"
#include "authenticator.h"

void setup_env() {
}

char before_test() {
  return 0;
}

char after_test() {
  return 0;
}

void clean_env() {
}

char test_auth(void) {
  printf("Adding user ze\n");
  result auth_result = add_user("ze", "ze");
  return auth_result.result;
}
 
test test_suite[] = {
  {
"Authenticate user Ze with password Ze", test_auth
  }
}; 

int main(int argc, char **argv) {
  run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  return 0;
}

