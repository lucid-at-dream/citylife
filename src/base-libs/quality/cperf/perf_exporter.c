#include <stdlib.h>

#include "perf_exporter.h"

void register_exporter(dynarray *exporters, void (*per_report)(perf_test, perf_report), void(*final_report)(void)) {
    exporter *e = (exporter *)malloc(sizeof(exporter));

    e->process_report = per_report;
    e->finalize = final_report;

    dynarray_add(exporters, e);
}

void process_report(dynarray *exporters, perf_test test, perf_report report) {
    for (int i = 0; i < exporters->size; i++) {
        ((exporter *)dynarray_get(exporters, i))->process_report(test, report);
    }
}

void finalize_report(dynarray *exporters) {
    for (int i = 0; i < exporters->size; i++) {
        ((exporter *)dynarray_get(exporters, i))->finalize();
        free(dynarray_get(exporters, i));
    }
}
