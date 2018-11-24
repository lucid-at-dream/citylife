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
} test;

/**
 * Function that will be executed before all the tests in the test suite.
 *
 * Suggestions: Setup the test environment
 */
void setup_env();

/**
 * Executes before each test is executed.
 *
 * Suggestions: Assert the environment is usable for the test.
 */
char before_test();

/**
 * Executes after each test is executed.
 * 
 * Suggestion: Perform generic clean up routines.
 */
char after_test();

/**
 * Executes after the entire test suite has executed.
 *
 * Suggestion: Tear everything down and free up allocated resources.
 */
void clean_env();

/**
 * Executes a test suite, received as an array of tests.
 *
 * @param test_suite An array of tests to be executed.
 * @param suite_size The number of tests in the test suite.
 * @return A report of the tests that were executed.
 */
suite_report run_test_suite(test *test_suite, int suite_size);

