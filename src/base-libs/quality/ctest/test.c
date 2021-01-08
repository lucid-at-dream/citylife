#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>

#include "test.h"

char runTest(test *t);
void display_report(suite_report *report);

suite_report run_test_suite(test *test_suite, int suite_size)
{
    suite_report report = { 0, 0, 0 }; // succeeded, failed, total

    int count = 0;
    for (; count < suite_size; count++) {
        test *t = test_suite + count;

        printf("========= Running test %d: '%s'\n", count + 1, t->description);

        pthread_t *thread = (pthread_t *)calloc(1, sizeof(pthread_t));
        pthread_create(thread, NULL, runTest, t);

        int exit_status;
        pthread_join(*thread, &exit_status);

        char failed;
        if (exit_status != t->expected_exit_status) {
            printf("Expected the test to have returned the value %d, got %d instead\n", t->expected_exit_status, exit_status);
            failed = 1;
        } else {
            failed = 0;
        }

        free(thread);

        printf("========= Finished executing test %d: %s (%s)\n", count + 1, t->description, failed ? "FAILURE" : "SUCCESS");

        report.successes += failed ? 0 : 1;
        report.failures += failed ? 1 : 0;
        report.total += 1;
    }

    display_report(&report);

    return report;
}

void display_report(suite_report *report)
{
    printf("Finished executing %d tests.\n", report->total);
    printf("Tests executed > Success: %d tests | Failure: %d tests\n", report->successes, report->failures);
}

/**
 * Executes the before test, test, and after test routines. If any of 
 * them fails then the test is considered as failed. Failures in one
 * of the functions does not prevent the others from running.
 *
 * @param t The test that will be executed
 * @return 0 if the test succeeded, something else on failure.
 */
char runTest(test *t)
{
    return t->test_impl();
}
