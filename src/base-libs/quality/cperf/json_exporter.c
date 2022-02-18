#include "json_exporter.h"

#include "perftest.h"

#include <stdio.h>

char **reports = NULL;

void save_report_json(perf_test test, perf_report report) {
    // Allocate memory for another report in the array
    int size = 0;
    if (reports != NULL) {
        size = sizeof(*reports) / sizeof(char *);
    }
    reports = realloc(reports, size + 1);

    // Build report's json text
    char *json = calloc(8096, sizeof(char));

    sprintf(json,
            "{\n"
            "  \"test\": \"%s\",\n"
            "  \"mean\": %lf,\n"
            "  \"stddev\": %lf,\n"
            "  \"stderr\": %lf,\n"
            "  \"p90\": %lf,\n"
            "  \"p95\": %lf,\n"
            "  \"p99\": %lf,\n"
            "  \"p999\": %lf,\n"
            "  \"p9999\": %lf,\n"
            "  \"p99999\": %lf,\n"
            "}\n",
            test.description, report.mean, report.standard_deviation, report.standard_error, report.p90, report.p95, report.p99, report.p999, report.p9999,
            report.p99999);

    // Save report's json in memory
    reports[size] = json;
}

void finalize_json_report() {
    int size = sizeof(*reports) / sizeof(char *);

    FILE *f = fopen("perf_report.json", "w");

    fwrite("[\n", sizeof(char), 2, f);

    for (int i = 0; i < size; i++) {
        fwrite(reports[i], sizeof(char), strnlen(reports[i], 8096), f);

        if (i != size - 1) {
            fwrite(",", sizeof(char), 2, f);
        }
    }

    fclose(f);
}
