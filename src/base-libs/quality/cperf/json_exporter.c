#include "json_exporter.h"

#include <stdio.h>
#include <time.h>

#include "dynarray.h"
#include "perftest.h"

dynarray *reports = NULL;

void save_report_json(perf_test test, perf_report report)
{
    // Build report's json text
    char *json = calloc(8096, sizeof(char));

    snprintf(
            json,
            8096 * sizeof(char),
            "  {\n"
            "    \"test\": \"%s\",\n"
            "    \"mean\": %lf,\n"
            "    \"stddev\": %lf,\n"
            "    \"stderr\": %lf,\n"
            "    \"p90\": %lf,\n"
            "    \"p95\": %lf,\n"
            "    \"p99\": %lf,\n"
            "    \"p999\": %lf,\n"
            "    \"p9999\": %lf,\n"
            "    \"p99999\": %lf,\n"
            "  }\n",
            test.description,
            report.mean,
            report.standard_deviation,
            report.standard_error,
            report.p90,
            report.p95,
            report.p99,
            report.p999,
            report.p9999,
            report.p99999);

    // Save report's json in memory
    if (reports == NULL)
    {
        reports = dynarray_new();
    }
    dynarray_add(reports, json);
}

void finalize_json_report()
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    char filename[128];
    snprintf(
            filename,
            128 * sizeof(char),
            "perf_report_%d%02d%02d_%02d%02d%02d.json",
            tm.tm_year + 1900,
            tm.tm_mon + 1,
            tm.tm_mday,
            tm.tm_hour,
            tm.tm_min,
            tm.tm_sec);

    FILE *f = fopen(filename, "w");

    fwrite("[\n", sizeof(char), 2, f);

    for (int i = 0; i < reports->size; i++)
    {
        char *report = dynarray_get(reports, i);
        fwrite(report, sizeof(char), strnlen(report, 8096), f);

        if (i != reports->size - 1)
        {
            fwrite(",", sizeof(char), 2, f);
        }
    }

    fwrite("]\n", sizeof(char), 2, f);

    fclose(f);
}
