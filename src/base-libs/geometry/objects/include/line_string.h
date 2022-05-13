#pragma once

#include "dynarray.h"

typedef struct _line_string {
    dynarray *vertex_list;
} line_string;

line_string *line_string_new();