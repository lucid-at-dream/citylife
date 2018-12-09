#include <stdio.h>
#include <string.h>
#include <time.h>

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

/** Test authenticator **/
char test_auth_add_user() {
  authenticator *auth = authenticator_new();  
  result auth_result = add_user(auth, "ze", "ze");
  authenticator_destroy(auth);

  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    return 1;
  }
  return 0;
}

char test_auth_add_same_user_twice() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, "ze", "ze");
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = add_user(auth, "ze", "manel");
  if (assert_int_equals("Adding a new user with a conflicting name results in error.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

/** Test map **/
char test_map_add_get() {
  char *ZE_NAME = "ze";
  char *ZE_PASS = "ze_";

  map *m = map_new(16);
  map_set(m, ZE_NAME, ZE_PASS);
  char *ze = map_get(m, ZE_NAME);

  if (ze == NULL) {
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

  map_set(m, ZE_NAME, ZE_PASS);
  map_set(m, MARIA_NAME, MARIA_PASS);
  
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

  map_set(m, ZE_NAME, ZE_PASS);
  map_set(m, ZE_DOP_NAME, ZE_DOP_PASS);
  
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

// TODO: This is a performance test, should be in a performance build.
char test_map_add_get_10000_elements_N_buckets_50_millis(int buckets) {
  
  int n_elements = 10000;
  char *NAME_PREFIX = "zemanel";
  char *PASS_PREFIX = "zemanel_";

  map *m = map_new(buckets);

  clock_t start = clock();

  for (int i = 0; i < n_elements; i++) {
    char user[12];
    char pass[12];

    sprintf(user, "%s%d", NAME_PREFIX, i);
    sprintf(pass, "%s%d", PASS_PREFIX, i);

    map_set(m, user, pass);
    char *retrieved_password = map_get(m, user);
  
    if (assert_str_equals("Retrieved password should equal added password", pass, retrieved_password)) {
      return 1;
    }
  }
  clock_t stop = clock();
  map_destroy(m);

  double time_taken = (stop - start) / (double)CLOCKS_PER_SEC;
  if (assert_float_less_than("Add/Get of 10K elements should take less than 10ms", time_taken, 0.050)) {
    return 1;
  }

  return 0;
}

char test_map_add_get_10000_elements_1_bucket_50_millis() {
  return test_map_add_get_10000_elements_N_buckets_50_millis(1);
}

char test_map_add_get_10000_elements_1000_buckets_50_millis() {
  return test_map_add_get_10000_elements_N_buckets_50_millis(1000);
}

char test_map_add_same_key_twice() {
  char *KEY = "key";
  char *VALUE_1 = "v1";
  char *VALUE_2 = "v2";

  map *m = map_new(1);

  map_set(m, KEY, VALUE_1);
  map_set(m, KEY, VALUE_2);
  
  char *result = map_get(m, KEY);
  
  map_destroy(m);

  if (assert_str_equals("The retrieved value is the last inserted for that key.", result, VALUE_2)) {
    return 1;
  }
  return 0;
}
 
test test_suite[] = {
  {
    "Add user Ze with password Ze to the authentication service", test_auth_add_user
  },
  {
    "Try to add a new user with a conflicting name", test_auth_add_same_user_twice
  },
  {
    "Test add functionality in maps", test_map_add_get
  },
  {
    "Test map add 2 elements", test_map_add_2_elements
  },
  {
    "Test adding 2 look alike elements to the map", test_map_add_2_doppleganger_elements
  },
  {
    "Test adding 10K users in a map with 1K bucket lists and retrieving in less than 50ms", 
    test_map_add_get_10000_elements_1000_buckets_50_millis
  },
  {
    "Test adding 10K users in a map with 1 bucket list and retrieving in less than 50ms",
    test_map_add_get_10000_elements_1_bucket_50_millis
  },
  {
    "Test adding two values for the same key. The last value inserted should be returned.",
    test_map_add_same_key_twice
  }
};

int main(int argc, char **argv) {
  suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  if (report.failures > 0) {
    return -1;
  }

  return 0;
}

