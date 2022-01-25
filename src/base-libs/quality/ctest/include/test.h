#pragma once

/**
 * Holds the report generated while running a test suite.
 */
typedef struct _suite_report {
    int successes;
    int failures;
    int total;
} suite_report;

/**
 * The test implementation and a verbose description displayed in the build logs.
 */
typedef struct _test {
    char *description;
    char (*test_impl)(void);
    int expected_exit_status;
} test;

/**
 * Executes a test suite, received as an array of tests.
 *
 * @param test_suite An array of tests to be executed.
 * @param suite_size The number of tests in the test suite.
 * @return A report of the tests that were executed.
 */
suite_report run_test_suite(test *test_suite, int suite_size);

/**
 * Utility macro to simplify writing unit test scripts.
 */
#define TEST_SUITE(tests...)                                                                                                                                   \
    test test_suite[] = { tests };                                                                                                                             \
    int main(int argc, char **argv) {                                                                                                                          \
        suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));                                                                   \
                                                                                                                                                               \
        if (report.failures > 0) {                                                                                                                             \
            return -1;                                                                                                                                         \
        }                                                                                                                                                      \
                                                                                                                                                               \
        return 0;                                                                                                                                              \
    }

#define TEST_CASE(name, body)                                                                                                                                  \
    char name() {                                                                                                                                              \
        int assertion_error = 0;                                                                                                                               \
        { body }                                                                                                                                               \
        return assertion_error;                                                                                                                                \
    }

/**
 * Utility macro to simplify writing unit test scripts.
 */
#define RUN_TEST(d, t, args...)                                                                                                                                \
    { d, t, args }
