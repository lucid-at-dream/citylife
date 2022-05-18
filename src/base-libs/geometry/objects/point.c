#include "point.h"

point *point_new(long double x, long double y)
{
    point *new = (point *)calloc(1, sizeof(point));
    new->x = x;
    new->y = y;
    return new;
}

void point_destroy(point *p)
{
    free(p);
}