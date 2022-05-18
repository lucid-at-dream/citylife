#include "assert.h"
#include "test.h"

#include "geometry.h"
#include "wkt.y.tab.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char test_parse_single_point(long double x, long double y)
{
    char assertion_error = 0;

    char buff[128] = "";
    sprintf(buff, "POINT(%LF %LF)", x, y);

    geometry *geom = geometry_from_wkt(buff);

    ASSERT_INT_EQUALS("Element parsed is a point", geom->type, POINT);
    ASSERT_LF_EQUALS("X coordinate is 1.0", geom->object.p->x, x, 0.0000001);
    ASSERT_LF_EQUALS("Y coordinate is 1.0", geom->object.p->y, y, 0.0000001);

    point_destroy(geom->object.p);
    free(geom);

    return assertion_error;
}

TEST_CASE(test_point_parsing, {
    assertion_error += test_parse_single_point(1, 2);
    assertion_error += test_parse_single_point(1233.5678, 682.1233);
    assertion_error += test_parse_single_point(876.3234, 1368.32173);
})

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

char test_parse_single_polygon(int external_ring_size, int internal_ring_count, ...)
{
    // Read the parameters and build the test case
    va_list argptr;
    va_start(argptr, internal_ring_count);

    point external_ring[external_ring_size];
    char buffer[4096] = "POLYGON((";

    for (int i = 0; i < external_ring_size; i++)
    {
        if (i > 0)
        {
            strcat(buffer, ",");
        }

        char point_buffer[128] = "";
        point p = va_arg(argptr, point);
        external_ring[i] = p;
        sprintf(point_buffer, "%LF %LF", p.x, p.y);
        strcat(buffer, point_buffer);
    }
    strcat(buffer, ")");

    point *internal_rings[internal_ring_count];
    int internal_rings_size[internal_ring_count];
    if (internal_ring_count > 0)
    {
        for (int i = 0; i < internal_ring_count; i++)
        {
            strcat(buffer, ",(");
            int ring_size = va_arg(argptr, int);
            internal_rings_size[i] = ring_size;
            point *internal_ring = (point *)calloc(ring_size, sizeof(point));
            for (int j = 0; j < ring_size; j++)
            {
                if (j > 0)
                {
                    strcat(buffer, ",");
                }
                internal_ring[j] = va_arg(argptr, point);
                char point_buffer[128] = "";
                sprintf(point_buffer, "%LF %LF", internal_ring[j].x, internal_ring[j].y);
                strcat(buffer, point_buffer);
            }
            internal_rings[i] = internal_ring;
        }
        strcat(buffer, ")");
    }
    strcat(buffer, ")");
    printf("%s\n", buffer);
    va_end(argptr);

    // Parse (Function under test)
    geometry *geom = geometry_from_wkt(buffer);

    int assertion_error = 0;

    ASSERT_INT_EQUALS("Element parsed is a line string", geom->type, POLYGON);

    ASSERT_INT_EQUALS("Linestring has the correct number of points", geom->object.pol->external_ring->vertex_list->size, external_ring_size);

    for (int i = 0; i < external_ring_size; i++)
    {
        point *p = dynarray_get(geom->object.pol->external_ring->vertex_list, i);
        ASSERT_LF_EQUALS("X coordinate is 1.0", p->x, external_ring[i].x, 0.0000001);
        ASSERT_LF_EQUALS("Y coordinate is 1.0", p->y, external_ring[i].y, 0.0000001);
    }

    for (int i = 0; i < internal_ring_count; i++)
    {
        line_string *ring = dynarray_get(geom->object.pol->internal_rings->line_string_list, i);

        ASSERT_INT_EQUALS("Linestring has the correct number of points", ring->vertex_list->size, internal_rings_size[i]);
        for (int j = 0; j < internal_rings_size[i]; j++)
        {
            point *p = dynarray_get(ring->vertex_list, j);
            ASSERT_LF_EQUALS("X coordinate is 1.0", p->x, internal_rings[i][j].x, 0.0000001);
            ASSERT_LF_EQUALS("Y coordinate is 1.0", p->y, internal_rings[i][j].y, 0.0000001);
        }
        free(internal_rings[i]);
    }

    polygon_destroy(geom->object.pol);
    free(geom);

    return assertion_error;
}

TEST_CASE(test_polygon_parsing, {
    assertion_error += test_parse_single_polygon(5, 0, (point){ 30, 10 }, (point){ 40, 40 }, (point){ 20, 40 }, (point){ 10, 20 }, (point){ 30, 10 });

    assertion_error += test_parse_single_polygon(5, 1, (point){ 30, 10 }, (point){ 40, 40 }, (point){ 20, 40 }, (point){ 10, 20 }, (point){ 30, 10 }, 3,
                                                 (point){ 30, 10 }, (point){ 40, 40 }, (point){ 20, 40 });
})

// TODO: Add coverage for the remaining data structures

TEST_SUITE(RUN_TEST("Test parsing points.", &test_point_parsing), RUN_TEST("Test parsing linestrings.", &test_line_string_parsing),
           RUN_TEST("Test parsing polygons.", &test_polygon_parsing), )
