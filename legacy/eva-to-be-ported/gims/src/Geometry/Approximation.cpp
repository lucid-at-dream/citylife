#include "Geometry.hpp"
#include "PolygonIntersection.hpp"
#include "DCEL.hpp"

/*functions*/
int rcmp(const void *a, const void *b);
int asdfcmp(const void *a, const void *b);
double angle_v(GIMS_Point &v1, GIMS_Point &v2);
char side(GIMS_Point &p1, GIMS_Point &v1, GIMS_Point &p2);
appr_intersection calcIntersection(GIMS_ConvexHullAproximation *A, GIMS_ConvexHullAproximation *B);

/*globals*/
GIMS_Point *map0;

/*return the area of the convex hull*/
double GIMS_ConvexHullAproximation::getArea()
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

/*return the amount of area of the approximation that is not common with the covered polygon*/
double GIMS_ConvexHullAproximation::getFalseArea()
{
        return this->falsearea;
}

bool GIMS_ConvexHullAproximation::isDisjoint(GIMS_BoundingBox *box)
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

bool GIMS_ConvexHullAproximation::isInside(GIMS_BoundingBox *box)
{
        for (int i = 0; i < N; i++) {
                if (!hull[i]->isInsideBox(box))
                        return false;
        }
        return true;
}

bool GIMS_ConvexHullAproximation::containsApproximation(GIMS_Approximation *appr)
{
        GIMS_ConvexHullAproximation *other = (GIMS_ConvexHullAproximation *)appr;
        for (int i = 0; i < other->N; i++) {
                if (!this->containsPoint(other->hull[i]))
                        return false;
        }
        return true;
}

bool GIMS_ConvexHullAproximation::containsPolygon(GIMS_Polygon *p)
{
        for (int i = 0; i < p->externalRing->size; i++) {
                for (int j = 0; j < p->externalRing->list[i]->size; j++) {
                        if (!this->containsPoint(p->externalRing->list[i]->list[j]))
                                return false;
                }
        }
        return true;
}

int mod(int n, int m)
{
        if (n >= 0)
                return n % m;
        return m + n % m;
}

bool GIMS_ConvexHullAproximation::containsPoint(GIMS_Point *pt)
{
        for (int i = 0; i < N; i++) {
                GIMS_Point *a = hull[mod(i, N)], *b = hull[mod(i + 1, N)];
                if (ccw(a, b, pt) > ERR_MARGIN)
                        return false;
        }
        return true;
}

appr_intersection GIMS_ConvexHullAproximation::intersection(GIMS_Approximation *other)
{
        return calcIntersection(this, (GIMS_ConvexHullAproximation *)other);
}

/*Build the convex hull based on a polygon*/
GIMS_ConvexHullAproximation::GIMS_ConvexHullAproximation(GIMS_Polygon *polygon)
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

        this->N = graham_scan(ch_points, count);

        this->hull = ch_points;

        this->falsearea = this->getArea() - polygon->area();
}

GIMS_ConvexHullAproximation::~GIMS_ConvexHullAproximation()
{
        if (this->hull != NULL)
                free(this->hull);
}

bool GIMS_ConvexHullAproximation::isDisjointFromApproximation(GIMS_Approximation *other)
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

appr_intersection calcIntersection(GIMS_ConvexHullAproximation *A, GIMS_ConvexHullAproximation *B)
{
        /*Create a polygon with the convex hull A*/
        GIMS_LineString *ringA = new GIMS_LineString(A->N + 1);
        memcpy(ringA->list, A->hull, A->N * sizeof(GIMS_Point *));
        ringA->list[A->N] = A->hull[0];
        ringA->size = A->N + 1;
        GIMS_MultiLineString *externalRingA = new GIMS_MultiLineString(1);
        externalRingA->append(ringA);
        GIMS_Polygon polygonA = GIMS_Polygon(externalRingA, NULL, false);
        polygonA.approximation = NULL;

        /*Create a polygon with the convex hull B*/
        GIMS_LineString *ringB = new GIMS_LineString(B->N + 1);
        memcpy(ringB->list, B->hull, B->N * sizeof(GIMS_Point *));
        ringB->list[B->N] = B->hull[0];
        ringB->size = B->N + 1;
        GIMS_MultiLineString *externalRingB = new GIMS_MultiLineString(1);
        externalRingB->append(ringB);
        GIMS_Polygon polygonB = GIMS_Polygon(externalRingB, NULL, false);
        polygonB.approximation = NULL;

        DCEL planargraph = buildPlanarGraph(&polygonA, &polygonB);

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

double angle_v(GIMS_Point &v1, GIMS_Point &v2)
{
        double dp = v1.x * v2.x + v1.y * v2.y, nv1 = sqrt(v1.x * v1.x + v1.y * v1.y),
               nv2 = sqrt(v2.x * v2.x + v2.y * v2.y), cosine = dp / (nv1 * nv2);
        return acos(cosine > 1 ? 1.0 : cosine < -1 ? -1.0 : cosine);
}

/*given a line defined by point (p1) and vector(v1) and a point (p2)
returns -1 if p2 is to the right of the line, 1 if is to the left and 0 if it is
on the line*/
char side(GIMS_Point &p1, GIMS_Point &v1, GIMS_Point &p2)
{
        GIMS_Point aux;
        aux.x = v1.x + p1.x;
        aux.y = v1.y + p1.y;

        double turn = ccw(&p1, &aux, &p2);
        if (turn < ERR_MARGIN)
                return -1;
        else if (turn > ERR_MARGIN)
                return 1;
        return 0;
}

int rcmp(const void *a, const void *b)
{
        return -asdfcmp(a, b);
}

int asdfcmp(const void *a, const void *b)
{
        GIMS_Point *p1 = *((GIMS_Point **)(a));
        GIMS_Point *p2 = *((GIMS_Point **)(b));

        double ang1 = atan2(p1->y - map0->y, map0->x - p1->x), ang2 = atan2(p2->y - map0->y, map0->x - p2->x);

        if (ang1 > ang2)
                return 1;
        if (ang1 < ang2)
                return -1;
        return 0;
}

/*ccw >0, cw <0, cl =0*/
double ccw(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3)
{
        return (p2->x - p1->x) * (p3->y - p1->y) - (p2->y - p1->y) * (p3->x - p1->x);
}

int graham_scan(GIMS_Point **map, int N)
{
        int i;

        /*find point with lowest y*/
        int min = 0;
        for (i = 0; i < N; i++)
                if (map[i]->y < map[min]->y)
                        min = i;

        /*swap point with lowest y with map[0]*/
        GIMS_Point *aux = map[min];
        map[min] = map[0];
        map[0] = aux;
        map0 = map[0];

        /*sort by angle with map[0]*/
        qsort(map + 1, N - 1, sizeof(GIMS_Point *), asdfcmp);

        /*Graham scan*/
        int M = 1;
        for (i = 2; i < N; i++) {
                while (ccw(map[M - 1], map[M], map[i % N]) >= 0) {
                        if (M > 1)
                                M -= 1;
                        else if (i == N)
                                break;
                        else
                                i += 1;
                }
                M++;
                aux = map[i % N];
                map[i % N] = map[M];
                map[M] = aux;
        }
        return M + 1;
}
