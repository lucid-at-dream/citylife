#include "assert.h"
#include "test.h"

#include "geometry.h"
#include "wkt.y.tab.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char test_parse_single_line_string(int size, ...)
{
    // Read the parameters and build the test case
    va_list argptr;
    va_start(argptr, size);

    point point_list[size];
    char buffer[1024] = "LINESTRING(";

    for (int i = 0; i < size; i++)
    {
        if (i > 0)
        {
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

    for (int i = 0; i < size; i++)
    {
        point *p = dynarray_get(geom->object.ls->vertex_list, i);
        ASSERT_LF_EQUALS("X coordinate is 1.0", p->x, point_list[i].x, 0.0000001);
        ASSERT_LF_EQUALS("Y coordinate is 1.0", p->y, point_list[i].y, 0.0000001);
    }

    line_string_destroy(geom->object.ls);
    free(geom);

    return assertion_error;
}

TEST_CASE(test_line_string_parsing, {
    assertion_error += test_parse_single_line_string(3, (point){ 5, 6 }, (point){ 3, 4 }, (point){ 7, 8 });

    assertion_error += test_parse_single_line_string(6, (point){ 5, 6 }, (point){ 3, 4 }, (point){ 7, 8 }, (point){ 321.123, 342 }, (point){ 432.321, 445.432 },
                                                     (point){ 10, 10 });

    assertion_error += test_parse_single_line_string(2, (point){ 5, 6 }, (point){ 3, 4 });
})

// TODO: Add negative cases (parse errors) and assert error message relevance

TEST_SUITE(RUN_TEST("Test parsing linestrings.", &test_line_string_parsing), )
