#pragma once

typedef struct _point {
    long double x, y;
} point;

point *point_new(long double x, long double y);
