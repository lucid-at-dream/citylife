#include "Geometry.hpp"

using namespace GIMS_GEOMETRY;

GIMS_Geometry::~GIMS_Geometry() {
}
GIMS_Approximation::~GIMS_Approximation() {
}

bool geometryIdCmp(GIMS_Geometry *A, GIMS_Geometry *B) {
    return A->id < B->id;
}

idset idIndex(&geometryIdCmp);

GIMS_Approximation *createPolygonApproximation(GIMS_Polygon *p) {
    switch (configuration.approximationType) {
    case 0:
        return new GIMS_MBR(p);

    case 1:
        return new GIMS_ConvexHullAproximation(p);

    default:
        return new GIMS_MBNgon(p);
    }
}

/*helper function*/
int dim(GIMS_Geometry *g) {
    switch (g->type) {
    case POINT:
    case MULTIPOINT:
        return 0;
    case LINESTRING:
    case LINESEGMENT:
    case RING:
    case MULTILINESTRING:
        return 1;
    case POLYGON:
    case MULTIPOLYGON:
    case BOUNDINGBOX:
        return 2;
    case GEOMETRYCOLLECTION:
        GIMS_GeometryCollection *gc = (GIMS_GeometryCollection *)g;
        int max = -1, aux;
        for (int i = 0; i < gc->size && max < 2; i++) {
            if ((aux = dim(gc->list[i])) > max)
                max = aux;
        }
        return max;
    }
    return -1;
}

int borderDim(GIMS_Geometry *g) {
    switch (g->type) {
    case RING: // a ring has no border
        return -1;
    case POINT:
    case MULTIPOINT:
        return 0;
    case LINESTRING:
    case LINESEGMENT:
    case MULTILINESTRING:
        return 0;
    case POLYGON:
    case MULTIPOLYGON:
    case BOUNDINGBOX:
        return 1;
    case GEOMETRYCOLLECTION:
        GIMS_GeometryCollection *gc = (GIMS_GeometryCollection *)g;
        int max = -1, aux;
        for (int i = 0; i < gc->size && max < 1; i++) {
            if ((aux = dim(gc->list[i])) > max)
                max = aux;
        }
        return max;
    }
    return -1;
}

GIMS_Geometry *fromWkt(char *wkt) {
    return lyWktParse(wkt);
}

bool collinear_3p(GIMS_Point *a, GIMS_Point *b, GIMS_Point *c) {
    double s = (b->x - a->x) * (c->y - a->y) - (b->y - a->y) * (c->x - a->x);
    if (fabs(s) < ERR_MARGIN)
        return true;
    return false;
}

/*returns the squared distance between two points*/
double distanceSquared2p(GIMS_Point *p1, GIMS_Point *p2) {
    return (p2->x - p1->x) * (p2->x - p1->x) + (p2->y - p1->y) * (p2->y - p1->y);
}

/*returns the cosine of the angle between vectors p1-p2 and p2-p3*/
double cosine3p(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3) {
    double SD12 = distanceSquared2p(p1, p2), SD13 = distanceSquared2p(p1, p3), SD23 = distanceSquared2p(p2, p3);

    return (SD12 + SD23 - SD13) / (2 * sqrt(SD23) * sqrt(SD12));
}

/*returns angle between vectors p1-p2 and p2-p3*/
double angle3p(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3) {
    return acos(cosine3p(p1, p2, p3));
}

/*Returns the squared distance between a point "p" and the closest point that
  lies in the given line segment "e". Note that the distance is squared to avoid
  possibly unnecessary square roots.*/
double distToSegmentSquared(GIMS_Point *p, GIMS_LineSegment *e) {
    //Length of the line segment squared
    double lineSegLenSquared = (e->p1->x - e->p2->x) * (e->p1->x - e->p2->x) + (e->p1->y - e->p2->y) * (e->p1->y - e->p2->y);

    if (lineSegLenSquared == 0)
        //if true then the line segment is actually a point.
        return (p->x - e->p1->x) * (p->x - e->p1->x) + (p->y - e->p1->y) * (p->y - e->p1->y);

    //dot product of (p - e->p1, e->p2 - e->p1)
    double dotProduct = (p->x - e->p1->x) * (e->p2->x - e->p1->x) + (p->y - e->p1->y) * (e->p2->y - e->p1->y);

    /*If we define the line that extends edge "e" as l(t) = e->p1 + t * (e->p2 - e->p1)
      The closest point of "p" that lies in line "l", is a point "cp" such that
      the line p(t) = p + t *(cp - p) forms a 90º degree angle with line "l".
      In order to find the value of t in l(t) for point cp, we have 
      t = [(p - e->p1).(e->p2 - e->p1)] / |e->p2 - e->p1|^2*/
    double t = dotProduct / lineSegLenSquared;
    if (t < 0) {
        //if t < 0 then cp lies beyond e->p1, and therefore e->p1 is the closest point
        return (p->x - e->p1->x) * (p->x - e->p1->x) + (p->y - e->p1->y) * (p->y - e->p1->y);
    }
    if (t > 1) {
        //if t < 0 then cp lies beyond e->p2, and therefore e->p2 is the closest point
        return (p->x - e->p2->x) * (p->x - e->p2->x) + (p->y - e->p2->y) * (p->y - e->p2->y);
    }

    //if t is in [0,1], then we must calculate point cp (nearest_pt)
    double nearest_pt_x = e->p1->x + t * (e->p2->x - e->p1->x), nearest_pt_y = e->p1->y + t * (e->p2->y - e->p1->y);

    //we then return the distance between point "p" and point cp
    return (p->x - nearest_pt_x) * (p->x - nearest_pt_x) + (p->y - nearest_pt_y) * (p->y - nearest_pt_y);
}
