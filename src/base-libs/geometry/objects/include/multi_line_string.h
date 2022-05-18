#pragma once

#include "dynarray.h"

typedef struct _multi_line_string
{
    dynarray *line_string_list;
} multi_line_string;

multi_line_string *multi_line_string_new();
void multi_line_string_destroy(multi_line_string *mls);