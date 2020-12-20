#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include <test.h>
#include <assert.h>
#include <map.h>

char *new_string(char *);

char test_map_add_get()
{
        char *ZE_NAME = new_string("ze");
        char *ZE_PASS = new_string("ze_");

        map *m = map_new(16);
        map_set(m, ZE_NAME, ZE_PASS);
        char *ze = map_get(m, ZE_NAME);

        char assertion_result = assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS);

        free(ZE_NAME);
        free(ZE_PASS);
        map_destroy(m);

        return assertion_result;
}

char test_map_get_empty_map()
{
        map *m = map_new(16);
        char *ze = map_get(m, "ze");
        map_destroy(m);

        return assert_null("Retrieving a value from an empty map should yield a NULL pointer.", ze);
}

char test_map_add_2_elements()
{
        char *ZE_NAME = new_string("ze");
        char *ZE_PASS = new_string("ze_");
        char *MARIA_NAME = new_string("maria");
        char *MARIA_PASS = new_string("maria_");

        map *m = map_new(16);

        map_set(m, ZE_NAME, ZE_PASS);
        map_set(m, MARIA_NAME, MARIA_PASS);

        char *ze = map_get(m, ZE_NAME);
        char *maria = map_get(m, MARIA_NAME);

        char assertion_result = assert_str_equals("Ze's password matches the inserted one.", ze, ZE_PASS) ||
                                assert_str_equals("Maria's password matches the inserted one.", maria, MARIA_PASS);

        free(ZE_NAME);
        free(ZE_PASS);
        free(MARIA_NAME);
        free(MARIA_PASS);
        map_destroy(m);

        return assertion_result;
}

char test_map_add_2_doppleganger_elements()
{
        char *ZE_NAME = new_string("zemanel");
        char *ZE_PASS = new_string("zemanel_");
        char *ZE_DOP_NAME = new_string("zemaneli");
        char *ZE_DOP_PASS = new_string("zemaneli_");

        map *m = map_new(16);

        map_set(m, ZE_NAME, ZE_PASS);
        map_set(m, ZE_DOP_NAME, ZE_DOP_PASS);

        char *zemanel = map_get(m, ZE_NAME);
        char *zemanel_dop = map_get(m, ZE_DOP_NAME);

        char assertion_result =
                assert_str_equals("Ze Manel's password matches the inserted one.", zemanel, ZE_PASS) ||
                assert_str_equals("Ze Maneli's password matches the inserted one.", zemanel_dop, ZE_DOP_PASS);

        free(ZE_NAME);
        free(ZE_PASS);
        free(ZE_DOP_NAME);
        free(ZE_DOP_PASS);
        map_destroy(m);

        return assertion_result;
}

char test_map_add_get_10000_elements_N_buckets_500_millis(int buckets)
{
        int n_elements = 10000;
        char *NAME_PREFIX = "zemanel";
        char *PASS_PREFIX = "zemanel_";

        char *users[n_elements];
        char *passes[n_elements];

        map *m = map_new(buckets);

        clock_t start = clock();

        for (int i = 0; i < n_elements; i++) {
                char *user = (char *)calloc(20, sizeof(char));
                char *pass = (char *)calloc(20, sizeof(char));

                users[i] = user;
                passes[i] = pass;

                snprintf(user, 20, "%s%d", NAME_PREFIX, i);
                snprintf(pass, 20, "%s%d", PASS_PREFIX, i);

                map_set(m, user, pass);
        }

        char lookup_user[20];
        char expected_pass[20];
        char assertion_result = 0;
        for (int i = 0; i < n_elements; i++) {
                snprintf(lookup_user, 20, "%s%d", NAME_PREFIX, i);
                snprintf(expected_pass, 20, "%s%d", PASS_PREFIX, i);

                char *retrieved_password = map_get(m, lookup_user);
                assertion_result |= assert_str_equals("Retrieved password should equal added password", expected_pass,
                                                      retrieved_password);
                if (assertion_result) {
                        break;
                }
        }

        clock_t stop = clock();
        double time_taken = ((double)(stop - start)) / CLOCKS_PER_SEC;

        assertion_result |=
                assert_float_less_than("Add/Get of 10K elements should take less than 500ms", time_taken, 0.5);

        for (int i = 0; i < n_elements; i++) {
                free(users[i]);
                free(passes[i]);
        }
        map_destroy(m);

        return assertion_result;
}

char test_map_add_get_10000_elements_1_bucket_500_millis()
{
        return test_map_add_get_10000_elements_N_buckets_500_millis(1);
}

char test_map_add_get_10000_elements_1000_buckets_500_millis()
{
        return test_map_add_get_10000_elements_N_buckets_500_millis(1000);
}

char test_map_add_same_key_twice()
{
        char *KEY = new_string("key");
        char *VALUE_1 = new_string("v1");
        char *VALUE_2 = new_string("v2");

        map *m = map_new(1);

        map_set(m, KEY, VALUE_1);
        map_set(m, KEY, VALUE_2);

        char *result = map_get(m, KEY);

        char assertion_result =
                assert_str_equals("The retrieved value is the last inserted for that key.", result, VALUE_2);

        map_destroy(m);
        free(KEY);
        free(VALUE_1);
        free(VALUE_2);

        return assertion_result;
}

char test_map_delete_user()
{
        char *CONST_KEY = "key";
        char *KEY = new_string(CONST_KEY);
        char *VALUE = new_string("v1");

        map *m = map_new(1);

        map_set(m, KEY, VALUE);
        char *result = map_get(m, CONST_KEY);

        if (assert_str_equals("Value is correctly inserted in the map.", result, VALUE)) {
                free(KEY);
                free(VALUE);
                map_destroy(m);
                return 1;
        }

        map_del(m, CONST_KEY);
        result = map_get(m, CONST_KEY);

        if (assert_str_equals("Value is correctly deleted from the map.", result, NULL)) {
                free(KEY);
                free(VALUE);
                map_destroy(m);
                return 1;
        }

        free(KEY);
        free(VALUE);
        map_destroy(m);
        return 0;
}

char test_map_delete_user_among_many_users()
{
        int n_keys = 1000;
        char *KEY_PREFIX = "k_";
        char *VALUE_PREFIX = "v_";

        char *keys[n_keys];
        char *values[n_keys];

        map *m = map_new(16);

        for (int i = 0; i < n_keys; i++) {
                char *key = (char *)malloc(10 * sizeof(char));
                char *value = (char *)malloc(10 * sizeof(char));

                snprintf(key, 10, "%s%d", KEY_PREFIX, i);
                snprintf(value, 10, "%s%d", VALUE_PREFIX, i);

                keys[i] = key;
                values[i] = value;

                map_set(m, key, value);
        }

        char *key_to_rm = (char *)calloc(10, sizeof(char));
        char *value_to_rm = (char *)calloc(10, sizeof(char));

        char assertion_result = 0;
        for (int i = 50; i < 1000; i += 50) {
                snprintf(key_to_rm, 10, "k_%d", i);
                snprintf(value_to_rm, 10, "v_%d", i);

                char *stored_value = map_get(m, key_to_rm);
                if (assertion_result |=
                    assert_str_equals("Value is correctly inserted in the map.", stored_value, value_to_rm)) {
                        break;
                }

                map_del(m, key_to_rm);
                stored_value = map_get(m, key_to_rm);
                if (assertion_result |=
                    assert_str_equals("Value is correctly deleted from the map.", stored_value, NULL)) {
                        break;
                }
        }

        free(key_to_rm);
        free(value_to_rm);

        for (int i = 0; i < n_keys; i++) {
                free(keys[i]);
                free(values[i]);
        }

        map_destroy(m);
        return assertion_result;
}

char test_map_delete_non_existing_user()
{
        map *m = map_new(1);

        map_set(m, "aa", "aa");
        map_set(m, "bb", "bb");
        map_set(m, "cc", "cc");

        map_del(m, "ze");

        map_destroy(m);

        return 0;
}

test test_suite[] = { { "Test add functionality in maps", test_map_add_get },
                      { "Test map get on empty map", test_map_get_empty_map },
                      { "Test map add 2 elements", test_map_add_2_elements },
                      { "Test adding 2 look alike elements to the map", test_map_add_2_doppleganger_elements },
                      { "Test adding 10K users in a map with 1K bucket lists and retrieving in less than 500ms",
                        test_map_add_get_10000_elements_1000_buckets_500_millis },
                      { "Test adding 10K users in a map with 1 bucket list and retrieving in less than 500ms",
                        test_map_add_get_10000_elements_1_bucket_500_millis },
                      { "Test adding two values for the same key. The last value inserted should be returned.",
                        test_map_add_same_key_twice },
                      { "Test deleting an existing user from a map", test_map_delete_user },
                      { "Test deleting some users among several other users", test_map_delete_user_among_many_users },
                      { "Test deleting a user that does not exist", test_map_delete_non_existing_user } };

int main(int argc, char **argv)
{
        suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

        if (report.failures > 0) {
                return -1;
        }

        return 0;
}

char *new_string(char *string)
{
        int max_size = 100;
        int buff_size = strlen(string) + 1 < max_size ? strlen(string) + 1 : max_size;
        char *ns = (char *)calloc(buff_size, sizeof(char));
        snprintf(ns, buff_size, "%s", string);
        return ns;
}
