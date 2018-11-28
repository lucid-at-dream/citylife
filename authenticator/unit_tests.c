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
  char *ZE_NAME = "ze";
  char *ZE_PASS = "ze_";

  map *m = map_new(16);
  map_add(m, ZE_NAME, ZE_PASS);
  char *ze = map_get(m, ZE_NAME);

  if (ze == NULL) {
    printf("Fuck, ze is null.");
    return 1;
  }

  map_destroy(m);

  if (assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS)) {
    return 1;
  }
  return 0;
}

char test_map_add_2_elements() {
  char *ZE_NAME = "ze";
  char *ZE_PASS = "ze_";
  char *MARIA_NAME = "maria";
  char *MARIA_PASS = "maria_";

  map *m = map_new(16);

  map_add(m, ZE_NAME, ZE_PASS);
  map_add(m, MARIA_NAME, MARIA_PASS);
  
  char *ze = map_get(m, ZE_NAME);
  char *maria = map_get(m, MARIA_NAME);
  
  map_destroy(m);

  if (assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS)) {
    return 1;
  }
  if (assert_str_equals("Maria's password matches the inserted one.", maria, MARIA_PASS)) {
    return 1;
  }
  return 0;
}

char test_map_add_2_doppleganger_elements() {
  char *ZE_NAME = "zemanel";
  char *ZE_PASS = "zemanel_";
  char *ZE_DOP_NAME = "zemaneli";
  char *ZE_DOP_PASS = "zemaneli_";

  map *m = map_new(16);

  map_add(m, ZE_NAME, ZE_PASS);
  map_add(m, ZE_DOP_NAME, ZE_DOP_PASS);
  
  char *zemanel = map_get(m, ZE_NAME);
  char *zemanel_dop = map_get(m, ZE_DOP_NAME);
  
  map_destroy(m);

  if (assert_str_equals("Ze Manel's password matches the inserted one.", zemanel, ZE_PASS)) {
    return 1;
  }
  if (assert_str_equals("Ze Maneli's password matches the inserted one.", zemanel_dop, ZE_DOP_PASS)) {
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
  },
  {
    "Test map add 2 elements", test_map_add_2_elements
  },
  {
    "Test adding 2 look alike elements to the map", test_map_add_2_doppleganger_elements
  } 
};

int main(int argc, char **argv) {
  run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  return 0;
}

