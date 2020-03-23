#include <stdio.h>
#include <stdlib.h>

#include "test.h"

int runTest(test *);
void display_report(suite_report *report);

suite_report run_test_suite(test *test_suite, int suite_size) {

  suite_report report = {0, 0, 0}; // succeeded, failed, total

  setup_env();

  int count = 0;
  for (; count < suite_size; count++) {
    test *t = test_suite + count;
    
    printf("========= Runing test %d: '%s'\n", count + 1, t->description);
    int failed = runTest(t);
    printf("========= Finished executing test %d: %s\n", count + 1, t->description);
    
    report.successes += failed ? 0 : 1;
    report.failures += failed ? 1 : 0;
    report.total += 1; 
  }

  clean_env();

  display_report(&report);


  return report;
}

void display_report(suite_report *report) {
  printf("Finished executing %d tests.\n", report->total);
  printf("Tests executed > Success: %d tests | Failure: %d tests\n", report-> successes, report->failures);
}

/**
 * Executes the before test, test, and after test routines. If any of 
 * them fails then the test is considered as failed. Failures in one
 * of the functions does not prevent the others from running.
 *
 * @param t The test that will be executed
 * @return 0 if the test succeeded, something else on failure.
 */
int runTest(test *t) {
  int failed = 0;
  failed |= before_test();
  failed |= t->test_impl();
  failed |= after_test();
  return failed;
}

