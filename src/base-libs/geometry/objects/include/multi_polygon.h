#pragma once

#include "dynarray.h"

typedef struct _multi_polygon
{
    dynarray *polygon_list;
} multi_polygon;

multi_polygon *multi_polygon_new();