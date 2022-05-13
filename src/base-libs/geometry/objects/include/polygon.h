#pragma once

#include "line_string.h"

typedef struct _polygon {
    line_string *external_ring;
    dynarray *internal_rings;
} polygon;

polygon *polygon_new(line_string *external_ring, dynarray *internal_rings);