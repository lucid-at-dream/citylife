#include <stdlib.h>

#include "perf_exporter.h"

exporter *register_exporter(exporter **exporters_ptr, void (*per_report)(perf_test, perf_report), void(*final_report)(void)) {

    exporter *exporters;
    if (exporters == NULL) {
        exporters = (exporter *)calloc(1, sizeof(exporter));
    } else {
        exporters = *exporters_ptr;
    }
    
    int i = sizeof(*exporters) / sizeof(exporter);

    exporter e = {
        per_report,
        final_report
    };

    exporters[i-1] = e;

    return exporters;
}

void process_report(exporter *exporters, perf_test test, perf_report report) {
    int size = sizeof(*exporters) / sizeof(exporter);

    for (int i = 0; i < size; i++) {
        exporters[i].process_report(test, report);
    }
}

void finalize_report(exporter *exporters) {
    int size = sizeof(*exporters) / sizeof(exporter);

    for (int i = 0; i < size; i++) {
        exporters[i].finalize();
    }
}
