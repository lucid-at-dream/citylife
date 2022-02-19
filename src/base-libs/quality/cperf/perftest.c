#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <math.h>

#include "perftest.h"
#include "perf_exporter.h"
#include "console_exporter.h"
#include "json_exporter.h"
#include "logger.h"
#include "dynarray.h"

typedef struct _iteration_report {
    double mean;
    double standard_deviation;
} iteration_report;

iteration_report *runPerfTest(perf_test *t);

int samples_count = 100;

dynarray *setup_exporters() {
    dynarray *exporters = dynarray_new();
    register_exporter(exporters, &export_to_console, &log_final_summary);
    register_exporter(exporters, &save_report_json, &finalize_json_report);
    return exporters;
}

void run_performance_test_suite(perf_test *test_suite, int suite_size) {
    dynarray *exporters = setup_exporters();

    int count = 0;
    for (; count < suite_size; count++) {
        perf_test *t = test_suite + count;

        printf("========= Running test %d: '%s'\n", count + 1, t->description);

        iteration_report *iteration_report;

        pthread_t *thread = (pthread_t *)calloc(1, sizeof(pthread_t));

        pthread_create(thread, NULL, runPerfTest, t);

        pthread_join(*thread, &iteration_report);

        free(thread);

        perf_report report;
        report.mean = iteration_report->mean;
        report.standard_deviation = iteration_report->standard_deviation;
        free(iteration_report);

        report.standard_error = report.standard_deviation / sqrt(samples_count);

        double z_p90 = 1.2816;
        report.p90 = z_p90 * report.standard_error;

        double z_p95 = 1.6449;
        report.p95 = z_p95 * report.standard_error;

        double z_p99 = 2.3263;
        report.p99 = z_p99 * report.standard_error;

        double z_p999 = 3.0902;
        report.p999 = z_p999 * report.standard_error;

        double z_p9999 = 3.719;
        report.p9999 = z_p9999 * report.standard_error;

        double z_p99999 = 4.2649;
        report.p99999 = z_p99999 * report.standard_error;

        printf("========= Finished executing test %d: %s\n", count + 1, t->description);

        process_report(exporters, *t, report);
    }

    finalize_report(exporters);
    dynarray_destroy(exporters);
}

/**
 * Executes the before test, test, and after test routines. If any of 
 * them fails then the test is considered as failed. Failures in one
 * of the functions does not prevent the others from running.
 *
 * @param t The test that will be executed
 * @return 0 if the test succeeded, something else on failure.
 */
iteration_report *runPerfTest(perf_test *t) {
    long elapsedNanos;
    struct timespec start, end;

    iteration_report *report = calloc(1, sizeof(iteration_report));

    long long int measurements[samples_count];

    for (int i = 0; i < samples_count; i++) {
        clock_gettime(CLOCK_MONOTONIC, &start);
        t->test_impl();
        clock_gettime(CLOCK_MONOTONIC, &end);

        long long int end_total_ns = end.tv_sec * 1000 * 1000 * 1000 + end.tv_nsec;
        long long int start_total_ns = start.tv_sec * 1000 * 1000 * 1000 + start.tv_nsec;

        measurements[i] = end_total_ns - start_total_ns;

        // Update mean
        report->mean = report->mean * ((double)i / (double)(i + 1)) + measurements[i] / (double)(i + 1);

        debug("Iteration %d | Measured %lld ns | Average %lf ns", i + 1, measurements[i], report->mean);
    }

    double differences_sum = 0;
    for (int i = 0; i < samples_count; i++) {
        differences_sum += (measurements[i] - report->mean) * (measurements[i] - report->mean);
    }

    report->standard_deviation = sqrt(differences_sum / (double)(samples_count - 1));

    return report;
}
