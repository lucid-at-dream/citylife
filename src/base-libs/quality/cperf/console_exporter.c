#include "console_exporter.h"

#include "logger.h"

void export_to_console(perf_test test, perf_report report) {
    info("= = = = = = Results for test %s = = = = = =", test.description);
    info("Mean: %.2lfns", report.mean);
    info("Standard Deviation: %.2lfns", report.standard_deviation);
    info("Standard Error of the Mean: %.2lfns", report.standard_error);
    info("P90: %.2lfns - %.2lfns", report.mean - report.p90, report.mean + report.p90);
    info("P95: %.2lfns - %.2lfns", report.mean - report.p95, report.mean + report.p95);
    info("P99: %.2lfns - %.2lfns", report.mean - report.p99, report.mean + report.p99);
    info("P999: %.2lfns - %.2lfns", report.mean - report.p999, report.mean + report.p999);
    info("P9999: %.2lfns - %.2lfns", report.mean - report.p9999, report.mean + report.p9999);
    info("P99999: %.2lfns - %.2lfns", report.mean - report.p99999, report.mean + report.p99999);
    info("= = = = = = End of Results = = = = = =");
}

void log_final_summary() {
    info("End of Benchmarks");
}