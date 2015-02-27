#include "Geometry.hpp"
#include <iostream>

using namespace GIMS_GEOMETRY;
using namespace std;

int main(){
    GIMS_Geometry *g = fromWkt("POLYGON((1 1, 2 1,2 3, 1 4, 1 1))");
    GIMS_Geometry *c = fromWkt("POINT(1 1)");
    GIMS_Geometry *a = fromWkt("LINESTRING(1 2, 3 4)");

    cout << g->toWkt() << endl << endl;
    cout << c->toWkt() << endl << endl;
    cout << a->toWkt() << endl << endl;

    GIMS_BoundingBox *box = ((GIMS_Polygon *)g)->getExtent();

    GIMS_Point *ll = box->lowerLeft,
               *ur = box->upperRight;

    printf("%lf %lf %lf %lf\n",ll->x, ll->y, ur->x, ur->y);

    box->deepDelete();
    g->deepDelete();
    c->deepDelete();

    return 0;
}
