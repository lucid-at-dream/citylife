
#include "bigint.h"
#include "../ctest/test.h"
#include "../ctest/assert.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

char test_single_word_bigint_add()
{
        bigint a = bigint_new(1000);
        bigint b = bigint_new(1000);

        bigint_add(a, b);

        char *result = bigint_tostring(a);

        int assertion_error = assert_str_equals("The two numbers were added correctly", result, "2000");

        // Clean up
        free(result);
        bigint_del(a);
        bigint_del(b);

        // Return test status
        if (assertion_error) {
                return 1;
        }
        return 0;
}

char test_single_word_bigint_increment()
{
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

test test_suite[] = { { "Test incrementing a single word big int", test_single_word_bigint_increment },
                      { "Test adding two single word big ints", test_single_word_bigint_add } };

int main(int argc, char **argv)
{
        suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

        if (report.failures > 0) {
                return -1;
        }

        return 0;
}
