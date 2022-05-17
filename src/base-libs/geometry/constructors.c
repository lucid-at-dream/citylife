#include "geometry.h"
#include "dynarray.h"

multi_point *multi_point_new() {
    multi_point *new = (multi_point *)calloc(1, sizeof(multi_point));
    new->point_list = dynarray_new();
    return new;
}

geometry_collection *geometry_collection_new() {
    geometry_collection *new = (geometry_collection *)calloc(1, sizeof(geometry_collection));
    new->geometry_list = dynarray_new();
    return new;
}

polygon *polygon_new(line_string *external_ring, multi_line_string *internal_rings) {
    polygon *new = (polygon *)calloc(1, sizeof(polygon));
    new->external_ring = external_ring;
    new->internal_rings = internal_rings;
    return new;
}

point *point_new(long double x, long double y) {
    point *new = (point *)calloc(1, sizeof(point));
    new->x = x;
    new->y = y;
    return new;
}

multi_polygon *multi_polygon_new() {
    multi_polygon *new = (multi_polygon *)calloc(1, sizeof(multi_polygon));
    new->polygon_list = dynarray_new();
    return new;
}

line_string *line_string_new() {
    line_string *new = (line_string *)calloc(1, sizeof(line_string));
    new->vertex_list = dynarray_new();
    return new;
}

geometry *geometry_new(geometry_type type, geometric_object object) {
    geometry *new = (geometry *)calloc(1, sizeof(geometry));
    new->type = type;
    new->object = object;
    return new;
}

multi_line_string *multi_line_string_new() {
    multi_line_string *new = (multi_line_string *)calloc(1, sizeof(multi_line_string));
    new->line_string_list = dynarray_new();
    return new;
}

