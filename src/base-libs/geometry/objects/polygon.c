#include "polygon.h"
#include <stdlib.h>

polygon *polygon_new(line_string *external_ring, multi_line_string *internal_rings) {
    polygon *new = (polygon *)calloc(1, sizeof(polygon));
    new->external_ring = external_ring;
    new->internal_rings = internal_rings;
    return new;
}

void polygon_destroy(polygon *pol) {

    if (pol->external_ring != NULL) {
        line_string_destroy(pol->external_ring);
    }

    if (pol->internal_rings != NULL) {
        multi_line_string_destroy(pol->internal_rings);
    }
    
    free(pol);
}
