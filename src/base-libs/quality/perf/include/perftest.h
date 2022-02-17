#pragma once

/**
 * The test implementation and a verbose description displayed in the build logs.
 */
typedef struct _perf_test {
    char *description;
    char (*test_impl)(void);
} perf_test;

/**
 * Executes a test suite, received as an array of tests.
 *
 * @param test_suite An array of tests to be executed.
 * @param suite_size The number of tests in the test suite.
 * @return A report of the tests that were executed.
 */
void run_performance_test_suite(perf_test *test_suite, int suite_size);

/**
 * Utility macro to simplify writing unit test scripts.
 */
#define PERF_TEST_SUITE(tests...)                                                           \
    perf_test test_suite[] = { tests };                                                     \
    int main(int argc, char **argv) {                                                       \
        run_performance_test_suite(test_suite, sizeof(test_suite) / sizeof(perf_test));          \
    }

#define PERF_TEST(name, body)\
    char name() {            \
        { body }             \
    }

/**
 * Utility macro to simplify writing unit test scripts.
 */
#define RUN_PERF_TEST(d, t) { d, t }
