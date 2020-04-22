
#include "bigint.h"
#include "ctest/test.h"
#include "ctest/assert.h"
#include <limits.h>
#include <stdio.h>

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

char test_bigint_increment() {
    bigint number = bigint_new(1000);

    bigint_increment(number);

    char *result = bigint_tostring(number);

    int assertion_error = assert_str_equals("Big int is incremented by one.", result, "1001");
    
    // Clean up
    free(result);
    bigint_del(number);

    // Return test status
    if (assertion_error) {
        return 1;
    }
    return 0;
}

test test_suite[] = {
    {
        "Test incrementing a big int", test_bigint_increment
    }
};

int main(int argc, char **argv) {
    suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
