#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "perftest.h"
#include "assert.h"
#include "map.h"

char test_map_add_get_10000_elements_N_buckets_1000_millis(int buckets) {
    int n_elements = 10000;
    char *NAME_PREFIX = "zemanel";
    char *PASS_PREFIX = "zemanel_";

    char *users[n_elements];
    char *passes[n_elements];

    map *m = map_new(buckets);

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
        assertion_result |= assert_str_equals("Retrieved password should equal added password", expected_pass, retrieved_password);
        if (assertion_result) {
            break;
        }
    }

    for (int i = 0; i < n_elements; i++) {
        free(users[i]);
        free(passes[i]);
    }
    map_destroy(m);

    return assertion_result;
}

PERF_TEST(test_map_add_get_10000_elements_1_buckets, { test_map_add_get_10000_elements_N_buckets_1000_millis(1); })

PERF_TEST(test_map_add_get_10000_elements_1000_buckets, { test_map_add_get_10000_elements_N_buckets_1000_millis(1000); })

PERF_TEST_SUITE(RUN_PERF_TEST("Test getting 10000 elements from a map starting with 1 bucket", &test_map_add_get_10000_elements_1_buckets),
                RUN_PERF_TEST("Test getting 10000 elements from a map starting with 1000 bucket", &test_map_add_get_10000_elements_1000_buckets), )
