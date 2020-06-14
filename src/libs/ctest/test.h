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
  char (*test_impl) (void);
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

