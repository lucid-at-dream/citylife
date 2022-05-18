#include "test.h"
#include "assert.h"

#include "wkt.y.tab.h"
#include "geometry.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

char test_parse_single_point(long double x, long double y) {
    char assertion_error = 0;

    char buff[128] = "";
    sprintf(buff, "POINT(%LF %LF)", x, y);

    geometry *geom = geometry_from_wkt(buff);

    ASSERT_INT_EQUALS("Element parsed is a point", geom->type, POINT);
    ASSERT_LF_EQUALS("X coordinate is 1.0", geom->object.p->x, x, 0.0000001);
    ASSERT_LF_EQUALS("Y coordinate is 1.0", geom->object.p->y, y, 0.0000001);

    free(geom->object.p);
    free(geom);

    return assertion_error;
}

TEST_CASE(test_point_parsing, {
    assertion_error += test_parse_single_point(1,2);
    assertion_error += test_parse_single_point(1233.5678,682.1233);
    assertion_error += test_parse_single_point(876.3234,1368.32173);
})

char test_parse_single_line_string(int size, ...) {
    
    // Read the parameters and build the test case
    va_list argptr;
    va_start(argptr, size);

    point point_list[size];
    char buffer[1024] = "LINESTRING(";

    for (int i = 0; i < size; i++) {
        if (i > 0) {
            strcat(buffer, ",");
        }
        
        char point_buffer[128] = "";
        point p = va_arg(argptr, point);
        point_list[i] = p;
        sprintf(point_buffer, "%LF %LF", p.x, p.y);
        strcat(buffer, point_buffer);
    }
    strcat(buffer, ")");
    va_end(argptr);

    // Parse (Function under test)
    geometry *geom = geometry_from_wkt(buffer);
    
    int assertion_error = 0;

    ASSERT_INT_EQUALS("Element parsed is a line string", geom->type, LINESTRING);

    ASSERT_INT_EQUALS("Linestring has the correct number of points", geom->object.ls->vertex_list->size, size);

    for (int i = 0; i < size; i++) {
        point *p = dynarray_get(geom->object.ls->vertex_list, i);
        ASSERT_LF_EQUALS("X coordinate is 1.0", p->x, point_list[i].x, 0.0000001);
        ASSERT_LF_EQUALS("Y coordinate is 1.0", p->y, point_list[i].y, 0.0000001);
        free(p);
    }

    dynarray_destroy(geom->object.ls->vertex_list);
    free(geom->object.ls);
    free(geom);

    return assertion_error;
}

TEST_CASE(test_line_string_parsing, {
    assertion_error += test_parse_single_line_string(3, (point){5, 6}, (point){3,4}, (point){7,8});
    
    assertion_error += test_parse_single_line_string(6, 
        (point){5, 6}, (point){3,4}, (point){7,8}, (point){321.123, 342}, (point){432.321,445.432}, (point){10,10}
    );

    assertion_error += test_parse_single_line_string(2, (point){5, 6}, (point){3,4});
})

// TODO: Add coverage for the remaining data structures

TEST_SUITE(
    RUN_TEST("Test parsing points.", &test_point_parsing),
    RUN_TEST("Test parsing linestrings.", &test_line_string_parsing),
)

