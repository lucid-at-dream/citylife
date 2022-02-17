#pragma once

#include "perftest.h"

typedef struct _exporter {
    void (*process_report) (perf_test test, perf_report report);
    void (*finalize) (void); 
} exporter; 

exporter *register_exporter(exporter **exporters, void (*)(perf_test, perf_report), void(*)(void));

void process_report(exporter *exporters, perf_test, perf_report);

void finalize_report(exporter *exporters);
