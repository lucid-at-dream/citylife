#include "Geometry.hpp"
#include "DCEL.hpp"
#include "PolygonIntersection.hpp"

appr_intersection calcIntersectionNGON_NGON(GIMS_MBNgon *A, GIMS_MBNgon *B);
int reduceHull(GIMS_Point **ch_points, int N);

bool GIMS_MBNgon::isDisjoint(GIMS_BoundingBox *box)
{
    for (int i = 0; i < N; i++) {
        GIMS_Point *p1 = hull[i], *p2 = hull[(i + 1) % N];
        GIMS_LineSegment l = GIMS_LineSegment(p1, p2);

        if (l.clipToBox(box) != NULL)
            return false;
    }

    if (hull[0]->isInsideBox(box))
        return false;
    return true;
}

bool GIMS_MBNgon::isInside(GIMS_BoundingBox *box)
{
    for (int i = 0; i < N; i++) {
        if (!hull[i]->isInsideBox(box))
            return false;
    }
    return true;
}

bool GIMS_MBNgon::isDisjointFromApproximation(GIMS_Approximation *other)
{
    bool found = false;
    for (int i = 0; i < this->N; i++) {
        found = true;
        GIMS_Point *p1 = this->hull[i], *p2 = this->hull[(i + 1) % N];
        GIMS_LineSegment ls = GIMS_LineSegment(p1, p2);

        for (int j = 0; j < other->N; j++) {
            if (other->hull[j]->sideOf(&ls) != LEFT)
                found = false;
        }

        if (found) {
            return true;
        }
    }
    return false;
}

bool GIMS_MBNgon::containsApproximation(GIMS_Approximation *appr)
{
    GIMS_ConvexHullAproximation *other = (GIMS_ConvexHullAproximation *)appr;
    for (int i = 0; i < other->N; i++) {
        if (!this->containsPoint(other->hull[i]))
            return false;
    }
    return true;
}

appr_intersection GIMS_MBNgon::intersection(GIMS_Approximation *other)
{
    return calcIntersectionNGON_NGON(this, (GIMS_MBNgon *)other);
}

double GIMS_MBNgon::getArea()
{
    GIMS_Point *p1 = hull[N - 1], *p2;
    double area = 0;
    for (int i = 0; i < N; i++) {
        p2 = hull[i];
        area += p1->x * p2->y - p2->x * p1->y;
        p1 = p2;
    }
    area = fabs(area / 2.0);
    return area;
}

double GIMS_MBNgon::getFalseArea()
{
    return falsearea;
}

GIMS_MBNgon::GIMS_MBNgon(GIMS_Polygon *polygon)
{
    GIMS_Point **ch_points = (GIMS_Point **)malloc(polygon->externalRing->getPointCount() * sizeof(GIMS_Point *));

    int count = 0;
    for (int r = 0; r < polygon->externalRing->size; r++) {
        GIMS_LineString *ring = polygon->externalRing->list[r];

        bool noGoToLast = ring->list[ring->size - 1]->equals(ring->list[0]) && ring->size > 1;

        for (int i = 0; i < ring->size - noGoToLast ? 1 : 0; i++) {
            ch_points[count++] = ring->list[i];
        }
    }

    /*calculate the convex hull using the graham scan algorithm*/
    this->N = graham_scan(ch_points, count);

    /*reduce the number of points in the hull to create an, at most, N-gon*/
    this->N = reduceHull(ch_points, this->N);

    this->hull = ch_points;
    this->falsearea = this->getArea() - polygon->area();
}

bool GIMS_MBNgon::containsPolygon(GIMS_Polygon *p)
{
    for (int i = 0; i < p->externalRing->size; i++) {
        for (int j = 0; j < p->externalRing->list[i]->size; j++) {
            if (!this->containsPoint(p->externalRing->list[i]->list[j]))
                return false;
        }
    }
    return true;
}

bool GIMS_MBNgon::containsPoint(GIMS_Point *pt)
{
    for (int i = 0; i < N; i++) {
        GIMS_Point *a = hull[mod(i, N)], *b = hull[mod(i + 1, N)];
        if (ccw(a, b, pt) > ERR_MARGIN)
            return false;
    }
    return true;
}

GIMS_MBNgon::~GIMS_MBNgon()
{
    if (this->hull)
        free(this->hull);
}

int reduceHull(GIMS_Point **ch_points, int N)
{
    /*if its already within the the size limits..*/
    if (N <= configuration.NGON_SIZE)
        return N;

    int min_index = 0;
    double min_value = 1e100;
    GIMS_Point min_intersection_point;

    for (int i = 0; i < N; i++) {
        //A and B are two edges separated by an edge..
        GIMS_Point *A_p1 = ch_points[mod(i - 1, N)], *A_p2 = ch_points[mod(i, N)], *B_p2 = ch_points[mod(i + 1, N)],
                   *B_p1 = ch_points[mod(i + 2, N)];

        mpf_class A_p1_x = A_p1->x, A_p1_y = A_p1->y, A_p2_x = A_p2->x, A_p2_y = A_p2->y, B_p1_x = B_p1->x,
                  B_p1_y = B_p1->y, B_p2_x = B_p2->x, B_p2_y = B_p2->y;

        mpf_class s1_x, s1_y, s2_x, s2_y;
        s1_x = A_p2_x - A_p1_x;
        s1_y = A_p2_y - A_p1_y;
        s2_x = B_p2_x - B_p1_x;
        s2_y = B_p2_y - B_p1_y;

        mpf_class s_den = (-s2_x * s1_y + s1_x * s2_y), s_num = (-s1_y * (A_p1_x - B_p1_x) + s1_x * (A_p1_y - B_p1_y)),
                  t_den = (-s2_x * s1_y + s1_x * s2_y), t_num = (s2_x * (A_p1_y - B_p1_y) - s2_y * (A_p1_x - B_p1_x));

        /*if the lines are parallel ignore.*/
        if ((s_den < ERR_MARGIN && s_den > -ERR_MARGIN) || (t_den < ERR_MARGIN && t_den > -ERR_MARGIN))
            continue;

        mpf_class s = (s_num / s_den), t = (t_num / t_den);

        if (s > 1 && t > 1) {
            mpf_class tmp_x = A_p1_x + (t * s1_x), tmp_y = A_p1_y + (t * s1_y);

            double int_x = tmp_x.get_d(), int_y = tmp_y.get_d();
            GIMS_Point int_p = GIMS_Point(int_x, int_y);

            //calculate the area of A_p2, B_p1, int_p
            GIMS_Point *p1 = A_p2, *p2 = &int_p, *p3 = B_p1;

            double area = fabs(p1->x * p2->y - p2->x * p1->y + p2->x * p3->y - p3->x * p2->y + p3->x * p1->y -
                               p1->x * p3->y) /
                          2.0;

            if (area < min_value) {
                min_value = area;
                min_index = i;
                min_intersection_point = int_p;
            }
        }
    }

    //remove the point hull[min_index] and hull[min_index+1] and replace them with their intersection.
    ch_points[min_index] = min_intersection_point.clone();
    for (int i = mod(min_index + 1, N); i < N - 1; i++)
        ch_points[i] = ch_points[i + 1];

    return reduceHull(ch_points, N - 1);
}

appr_intersection calcIntersectionNGON_NGON(GIMS_MBNgon *A, GIMS_MBNgon *B)
{
    /*Create a polygon with appr A*/
    GIMS_LineString *ringA = new GIMS_LineString(A->N + 1);
    memcpy(ringA->list, A->hull, A->N * sizeof(GIMS_Point *));
    ringA->list[A->N] = A->hull[0];
    ringA->size = A->N + 1;
    GIMS_MultiLineString *externalRingA = new GIMS_MultiLineString(1);
    externalRingA->append(ringA);
    GIMS_Polygon *polygonA = new GIMS_Polygon(externalRingA, NULL, false);
    polygonA->approximation = NULL;

    /*Create a polygon with appr B*/
    GIMS_LineString *ringB = new GIMS_LineString(B->N + 1);
    memcpy(ringB->list, B->hull, B->N * sizeof(GIMS_Point *));
    ringB->list[B->N] = B->hull[0];
    ringB->size = B->N + 1;
    GIMS_MultiLineString *externalRingB = new GIMS_MultiLineString(1);
    externalRingB->append(ringB);
    GIMS_Polygon *polygonB = new GIMS_Polygon(externalRingB, NULL, false);
    polygonB->approximation = NULL;

    DCEL planargraph = buildPlanarGraph(polygonA, polygonB);

    /*given the planar graph, go through its data and calculate the intersection.*/
    appr_intersection its = { false, 0 };

    for (facelist::iterator it = planargraph.faces.begin(); it != planargraph.faces.end(); it++) {
        face *f = *it;
        if (isFaceOfA(f) && isFaceOfB(f)) {
            its.intersects = true;
            its.area += f->getArea();
        }
    }

    for (vertexlist::iterator it = planargraph.vertexes.begin(); it != planargraph.vertexes.end(); it++) {
        vertex *v = *it;
        if (isRealVertexOfA(v) && isRealVertexOfB(v)) {
            its.intersects = true;
        }
    }

    for (halfedgelist::iterator it = planargraph.halfedges.begin(); it != planargraph.halfedges.end(); it++) {
        halfedge *he = *it;

        if (isExteriorEdgeOfA(he) && isExteriorEdgeOfB(he)) {
            its.intersects = true;
        }
    }

    return its;
}
