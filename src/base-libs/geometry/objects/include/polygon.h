#pragma once

#include "line_string.h"
#include "multi_line_string.h"

typedef struct _polygon
{
    line_string *external_ring;
    multi_line_string *internal_rings;
} polygon;

polygon *polygon_new(line_string *external_ring, multi_line_string *internal_rings);
void polygon_destroy(polygon *pol);