#pragma once

#include "dynarray.h"
#include "perftest.h"

typedef struct _exporter
{
    void (*process_report)(perf_test test, perf_report report);
    void (*finalize)(void);
} exporter;

void register_exporter(dynarray *exporters, void (*per_report)(perf_test, perf_report), void (*final_report)(void));
void process_report(dynarray *exporters, perf_test test, perf_report report);
void finalize_report(dynarray *exporters);
