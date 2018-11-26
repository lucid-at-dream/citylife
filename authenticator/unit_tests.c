#include <stdio.h>
#include <string.h>

#include "test.h"
#include "authenticator.h"
#include "map.h"
#include "assert.h"

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

char test_map_add_get() {
  map *m = map_new(16);
  map_add(m, "ze", "ze");
  char *ze = map_get(m, "ze");
  
  if (assert_str_equals("Ze's password matches the inserted one.", ze, "ze")) {
    return 1;
  }
  return 0;
}

char test_auth(void) {
  printf("Adding user ze\n");
  result auth_result = add_user("ze", "ze");
  return auth_result.result;
}
 
test test_suite[] = {
  {
    "Authenticate user Ze with password Ze", test_auth
  },
  {
    "Test add functionality in maps", test_map_add_get
  }
}; 

int main(int argc, char **argv) {
  run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  return 0;
}

