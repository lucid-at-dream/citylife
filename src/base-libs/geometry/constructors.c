#include "dynarray.h"
#include "geometry.h"

#include <stdlib.h>

multi_point *multi_point_new()
{
    multi_point *new = (multi_point *)calloc(1, sizeof(multi_point));
    new->point_list = dynarray_new();
    return new;
}

geometry_collection *geometry_collection_new()
{
    geometry_collection *new = (geometry_collection *)calloc(1, sizeof(geometry_collection));
    new->geometry_list = dynarray_new();
    return new;
}

multi_polygon *multi_polygon_new()
{
    multi_polygon *new = (multi_polygon *)calloc(1, sizeof(multi_polygon));
    new->polygon_list = dynarray_new();
    return new;
}

geometry *geometry_new(geometry_type type, geometric_object object)
{
    geometry *new = (geometry *)calloc(1, sizeof(geometry));
    new->type = type;
    new->object = object;
    return new;
}
