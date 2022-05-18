#include "multi_line_string.h"
#include "line_string.h"

multi_line_string *multi_line_string_new()
{
    multi_line_string *new = (multi_line_string *)calloc(1, sizeof(multi_line_string));
    new->line_string_list = dynarray_new();
    return new;
}

void multi_line_string_destroy(multi_line_string *mls)
{
    for (int i = 0; i < mls->line_string_list->size; i++)
    {
        line_string_destroy(dynarray_get(mls->line_string_list, i));
    }
    dynarray_destroy(mls->line_string_list);
    free(mls);
}