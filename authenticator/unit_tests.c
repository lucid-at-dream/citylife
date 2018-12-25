#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "test.h"
#include "authenticator.h"
#include "map.h"
#include "assert.h"

char *new_string(char *);

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
  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  authenticator_destroy(auth);

  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    return 1;
  }
  return 0;
}

char test_auth_add_same_user_twice() {
  authenticator *auth = authenticator_new();

  char *ZE_NAME = new_string("ze");
  char *ZE_PASS = new_string("ze");
  char *ZE_PASS_OTHER = new_string("manel");

  result auth_result = add_user(auth, ZE_NAME, ZE_PASS);
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = add_user(auth, ZE_NAME, ZE_PASS_OTHER);
  if (assert_int_equals("Adding a new user with a conflicting name results in error.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }
  free(ZE_PASS_OTHER);

  authenticator_destroy(auth);
  return 0;
}

char test_auth_do_auth() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

char test_auth_do_auth_wrong_password() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "maria");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

char test_auth_do_auth_change_password() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  char *new_pass = new_string("maria");

  auth_result = change_password(auth, "ze", "maria", new_pass);
  if (assert_int_equals("Password for user ze is changed with success.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = change_password(auth, "ze", "ze", new_pass);
  if (assert_int_equals("Password for user ze is changed with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "maria");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

char test_auth_do_auth_delete_user() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = del_user(auth, "ze", "ze");
  if (assert_int_equals("User ze is deleted with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("Authentication with deleted user fails.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

char test_auth_do_auth_delete_user_wrong_pass() {
  authenticator *auth = authenticator_new();

  result auth_result = add_user(auth, new_string("ze"), new_string("ze"));
  if (assert_int_equals("New user is created with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("User ze is authenticated with success.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = del_user(auth, "ze", "maria");
  if (assert_int_equals("User ze is deleted with success.", auth_result.result, AUTH_ERROR)) {
    authenticator_destroy(auth);
    return 1;
  }

  auth_result = authenticate(auth, "ze", "ze");
  if (assert_int_equals("Authentication with deleted user fails.", auth_result.result, AUTH_SUCCESS)) {
    authenticator_destroy(auth);
    return 1;
  }

  authenticator_destroy(auth);
  return 0;
}

/** Test map **/
char test_map_add_get() {
  char *ZE_NAME = new_string("ze");
  char *ZE_PASS = new_string("ze_");

  map *m = map_new(16);
  map_set(m, ZE_NAME, ZE_PASS);
  char *ze = map_get(m, ZE_NAME);

  if (ze == NULL) {
    return 1;
  }

  if (assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS)) {
    map_destroy(m);
    return 1;
  }
  map_destroy(m);
  return 0;
}

char test_map_add_2_elements() {
  char *ZE_NAME = new_string("ze");
  char *ZE_PASS = new_string("ze_");
  char *MARIA_NAME = new_string("maria");
  char *MARIA_PASS = new_string("maria_");

  map *m = map_new(16);

  map_set(m, ZE_NAME, ZE_PASS);
  map_set(m, MARIA_NAME, MARIA_PASS);
  
  char *ze = map_get(m, ZE_NAME);
  char *maria = map_get(m, MARIA_NAME);
  
  if (assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS)) {
    map_destroy(m);
    return 1;
  }
  if (assert_str_equals("Maria's password matches the inserted one.", maria, MARIA_PASS)) {
    map_destroy(m);
    return 1;
  }

  map_destroy(m);
  return 0;
}

char test_map_add_2_doppleganger_elements() {
  char *ZE_NAME = new_string("zemanel");
  char *ZE_PASS = new_string("zemanel_");
  char *ZE_DOP_NAME = new_string("zemaneli");
  char *ZE_DOP_PASS = new_string("zemaneli_");

  map *m = map_new(16);

  map_set(m, ZE_NAME, ZE_PASS);
  map_set(m, ZE_DOP_NAME, ZE_DOP_PASS);
  
  char *zemanel = map_get(m, ZE_NAME);
  char *zemanel_dop = map_get(m, ZE_DOP_NAME);
  
  if (assert_str_equals("Ze Manel's password matches the inserted one.", zemanel, ZE_PASS)) {
    map_destroy(m);
    return 1;
  }
  if (assert_str_equals("Ze Maneli's password matches the inserted one.", zemanel_dop, ZE_DOP_PASS)) {
    map_destroy(m);
    return 1;
  }

  map_destroy(m);
  return 0;
}

char test_map_add_get_10000_elements_N_buckets_50_millis(int buckets) {
  
  int n_elements = 10000;
  char *NAME_PREFIX = "zemanel";
  char *PASS_PREFIX = "zemanel_";

  map *m = map_new(buckets);

  clock_t start = clock();

  for (int i = 0; i < n_elements; i++) {
    char *user = (char *)calloc(20, sizeof(char));
    char *pass = (char *)calloc(20, sizeof(char));

    sprintf(user, "%s%d", NAME_PREFIX, i);
    sprintf(pass, "%s%d", PASS_PREFIX, i);

    map_set(m, user, pass);
  }

  char lookup_user[20];
  char expected_pass[20];
  for (int i = 0; i < n_elements; i++) {

    sprintf(lookup_user, "%s%d", NAME_PREFIX, i);
    sprintf(expected_pass, "%s%d", PASS_PREFIX, i);

    char *retrieved_password = map_get(m, lookup_user);
    if (assert_str_equals("Retrieved password should equal added password", expected_pass, retrieved_password)) {
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
  char *KEY = new_string("key");
  char *VALUE_1 = new_string("v1");
  char *VALUE_2 = new_string("v2");

  map *m = map_new(1);

  map_set(m, KEY, VALUE_1);
  map_set(m, KEY, VALUE_2);
  
  char *result = map_get(m, KEY);
  
  if (assert_str_equals("The retrieved value is the last inserted for that key.", result, VALUE_2)) {
    map_destroy(m);
    return 1;
  }
  map_destroy(m);
  return 0;
}

char test_map_delete_user() {
  char *CONST_KEY = "key";
  char *KEY = new_string(CONST_KEY);
  char *VALUE = new_string("v1");

  map *m = map_new(1);

  map_set(m, KEY, VALUE);
  char *result = map_get(m, CONST_KEY);
  
  if (assert_str_equals("Value is correctly inserted in the map.", result, VALUE)) {
    map_destroy(m);
    return 1;
  }

  map_del(m, CONST_KEY);
  result = map_get(m, CONST_KEY);
  
  if (assert_str_equals("Value is correctly deleted from the map.", result, NULL)) {
    map_destroy(m);
    return 1;
  }

  map_destroy(m);
  return 0;
}

char test_map_delete_user_among_many_users() {
  int n_keys = 1000;
  char *KEY_PREFIX = "k_";
  char *VALUE_PREFIX = "v_";

  map *m = map_new(16);

  for (int i = 0; i < n_keys; i++) {
    char *key = (char *)calloc(10, sizeof(char));
    char *value = (char *)calloc(10, sizeof(char));

    sprintf(key, "%s%d", KEY_PREFIX, i);
    sprintf(value, "%s%d", VALUE_PREFIX, i);

    map_set(m, key, value);
  }

  char *key_to_rm = (char *)calloc(10, sizeof(char));
  char *value_to_rm = (char *)calloc(10, sizeof(char));

  for (int i = 50; i < 1000; i += 50) {

    sprintf(key_to_rm, "k_%d", i);
    sprintf(value_to_rm, "v_%d", i);
  
    char *stored_value = map_get(m, key_to_rm);
    if (assert_str_equals("Value is correctly inserted in the map.", stored_value, value_to_rm)) {
      map_destroy(m);
      return 1;
    }

    map_del(m, key_to_rm);
    stored_value = map_get(m, key_to_rm);
    if (assert_str_equals("Value is correctly deleted from the map.", stored_value, NULL)) {
      map_destroy(m);
      return 1;
    }
  }

  free(key_to_rm);
  free(value_to_rm);

  map_destroy(m);
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
    "Create an user and authenticate with it", test_auth_do_auth
  },
  {
    "Create an user and provide a wrong password for authentication", test_auth_do_auth_wrong_password
  },
  {
    "Test changing the password of an existing user", test_auth_do_auth_change_password
  },
  {
    "Test deleting an existing user", test_auth_do_auth_delete_user
  },
  {
    "Test that existing user is not deleted if wrong pass is provided", test_auth_do_auth_delete_user_wrong_pass
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
  },
  {
    "Test deleting an existing user from a map", test_map_delete_user
  },
  {
    "Test deleting some users among several other users", test_map_delete_user_among_many_users
  }
};

int main(int argc, char **argv) {
  suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

  if (report.failures > 0) {
    return -1;
  }

  return 0;
}

char *new_string(char *string) {
  char *ns = (char *)calloc(strlen(string) + 1, sizeof(char));
  sprintf(ns, "%s", string);
  return ns;
}

