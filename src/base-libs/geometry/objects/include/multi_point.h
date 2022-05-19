#pragma once

#include "dynarray.h"

typedef struct _multi_point
{
    dynarray *point_list;
} multi_point;

multi_point *multi_point_new();
void multi_point_destroy(multi_point *mp);