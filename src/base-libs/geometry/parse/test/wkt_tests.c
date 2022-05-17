#include "wkt.y.tab.h"

#include <stdlib.h>

int main() {
    geometry_from_wkt("POINT(1.0 2.0)");
}