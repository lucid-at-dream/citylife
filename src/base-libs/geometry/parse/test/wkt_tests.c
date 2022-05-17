#include "test.h"
#include "assert.h"

#include "wkt.y.tab.h"
#include "geometry.h"

#include <stdlib.h>
#include <stdio.h>

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

// TODO: Add coverage for the remaining data structures

TEST_SUITE(
    RUN_TEST("Test parsing points.", &test_point_parsing),
)

