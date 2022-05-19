#include "multi_point.h"
#include "dynarray.h"
#include "point.h"

#include <stdlib.h>

multi_point *multi_point_new()
{
    multi_point *new = (multi_point *)calloc(1, sizeof(multi_point));
    new->point_list = dynarray_new();
    return new;
}

void multi_point_destroy(multi_point *mp)
{
    for (int i = 0; i < mp->point_list->size; i++)
    {
        point_destroy(dynarray_get(mp->point_list, i));
    }
    dynarray_destroy(mp->point_list);
    free(mp);
}
