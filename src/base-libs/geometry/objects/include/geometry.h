#pragma once

#include "point.h"
#include "multi_point.h"
#include "line_string.h"
#include "multi_line_string.h"
#include "polygon.h"
#include "multi_polygon.h"
#include "geometry_collection.h"

typedef enum _geometry_type {
    // envolving polygon (aka envelope)
    BOUNDINGBOX = 1 << 0,
    
    //Elementary datatypes
    POINT = 1 << 1,
    LINESTRING = 1 << 2,
    RING = 1 << 3, //(a line string that is closed)
    POLYGON = 1 << 4,

    //Multi datatypes
    MULTIPOINT = 1 << 5,
    MULTILINESTRING = 1 << 6,
    MULTIPOLYGON = 1 << 7,
    
    //collections
    GEOMETRYCOLLECTION = 1 << 8
} geometry_type;

typedef union _geometric_object {
    point *p;
    multi_point *mp;
    line_string *ls;
    multi_line_string *mls;
    polygon *pol;
    multi_polygon *mpol;
    geometry_collection *gc;
} geometric_object;

typedef struct _geometry {
    geometry_type type;
    geometric_object object;
} geometry;

geometry *geometry_new();