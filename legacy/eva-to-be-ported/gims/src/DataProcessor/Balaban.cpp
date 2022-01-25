#include "Balaban.hpp"

using namespace ivbalaban;

//global variables
double skewness; //factor of skewness
xevent xevents[MAX_N]; //list of x coordinate events
endpoint endpoints[MAX_N]; //list of endpoints
GIMS_Point points[MAX_N];
double sweepline_x; //x value of the current sweepline
double vstrip[2] = { 0, 0 }; //x interval that defines the current vertical strip under analysis
iset intersections(&geom_set_cmp); //set of intersections

list<GIMS_Geometry *> ivbalaban::balaban(GIMS_MultiLineString *A, GIMS_MultiLineString *B) {
    //printf("Beggining balaban's algorithm\n");

    GIMS_MultiLineString *input[2] = { A, B };

    //find the number of line segments
    int N = 0;
    for (GIMS_MultiLineString *mls : input)
        for (int i = 0; i < mls->size; i++)
            N += mls->list[i]->size - 1;

    //cout << "BEGIN INPUT\n";
    //cout << A->toWkt() << endl;
    //cout << B->toWkt() << endl;
    //cout << "END INPUT\n";

    int count = 0;
    double minposskew = 1e10;
    for (GIMS_MultiLineString *mls : input) {
        for (int i = 0; i < mls->size; i++) {
            for (int j = 0; j < mls->list[i]->size - 1; j++) {
                double x1 = mls->list[i]->list[j]->x, y1 = mls->list[i]->list[j]->y, x2 = mls->list[i]->list[j + 1]->x, y2 = mls->list[i]->list[j + 1]->y;

                //printf("in: %lf %lf %lf %lf\n",x1,y1,x2,y2);

                points[count].x = x1;
                points[count].y = y1;
                points[count + 1].x = x2;
                points[count + 1].y = y2;
                points[count].id = points[count + 1].id = mls->id;
                count += 2;

                double forbiddenskew = y1 != y2 ? (x1 - x2) / (y2 - y1) : 1e10;
                if (forbiddenskew > 0.0001 && forbiddenskew < minposskew)
                    minposskew = forbiddenskew;
            }
        }
    }

    //apply skewness
    skewness = MIN(minposskew / 2.0, 0.5);
    for (int i = 0; i < 2 * N; i++)
        applySkewness(points[i]);

    //create endpoints
    for (int i = 0; i < 2 * N; i += 2) {
        GIMS_Point *p1, *p2;
        if (points[i].x < points[i + 1].x || (points[i].x == points[i + 1].x && points[i].y < points[i + 1].y)) {
            p1 = points + i;
            p2 = points + i + 1;
        } else {
            p2 = points + i;
            p1 = points + i + 1;
        }
        GIMS_LineSegment l = GIMS_LineSegment(p1, p2);
        l.id = p1->id;

        //create endpoint
        endpoints[i].type = LEFT_ENDPOINT;
        endpoints[i + 1].type = RIGHT_ENDPOINT;
        endpoints[i].ls = endpoints[i + 1].ls = l;
        endpoints[i].pt = p1;
        endpoints[i + 1].pt = p2;
    }

    //sort endpoints
    qsort(endpoints, 2 * N, sizeof(endpoint), endpt_cmp);

    //merge endpoints with the same x coordinate in the same event
    count = 0;
    for (int i = 0; i < 2 * N; i++) {
        if (count > 0 && xevents[count - 1].endpoints[0]->pt->x == endpoints[i].pt->x) {
            xevents[count - 1].endpoints[xevents[count - 1].nevents] = endpoints + i;
            xevents[count - 1].nevents++;
        } else {
            xevents[count].endpoints[0] = endpoints + i;
            xevents[count].nevents = 1;
            count++;
        }
    }

    //initialize left sweepline intersecting line segments set
    oset L_r(&ls_cmp);
    for (int i = 0; i < xevents[0].nevents; i++)
        L_r.insert(xevents[0].endpoints[i]->ls);

    //add all other line segments to the internal line segments list
    uset I_r(&ls_set_cmp);
    for (int i = 0; i < count; i++)
        for (int j = 0; j < xevents[i].nevents; j++)
            I_r.insert(xevents[i].endpoints[j]->ls);

    //remove extra lines from internal line segments list
    for (int i = 0; i < xevents[0].nevents; i++)
        I_r.erase(xevents[0].endpoints[i]->ls);
    for (int i = 0; i < xevents[count - 1].nevents; i++)
        I_r.erase(xevents[count - 1].endpoints[i]->ls);

    oset R_r(&ls_cmp);

#ifdef DEBUG
    printf("L_r:\n");
    printoset(L_r);
    printf("I_r:\n");
    printuset(I_r);
    printf("R_r:\n");
    printoset(R_r);
#endif

    TreeSearch(L_r, I_r, 0, count - 1, R_r);

    list<GIMS_Geometry *> ret = list<GIMS_Geometry *>();
    for (iset::iterator it = intersections.begin(); it != intersections.end(); it++) {
        GIMS_Geometry *p = revertSkewness(*it);
        delete *it;
        ret.push_back(p);
    }

    intersections.clear();

    return ret;
}

void ivbalaban::setOrderReferenceLine(double x) {
    sweepline_x = x;
}

void ivbalaban::setStrip(double b, double e) {
    vstrip[0] = b;
    vstrip[1] = e;
}

//Balaban's algorithm main recursive function.
//complexity: O( Nlog^2(N) + K )
void ivbalaban::TreeSearch(oset &L_v, uset &I_v, int b, int e, oset &R_v) {
    double bx = xevents[b].endpoints[0]->pt->x, ex = xevents[e].endpoints[0]->pt->x;
#ifdef DEBUG
    printf("\n============\n");
    printf("BEGIN\nvertical strip: %lf %lf - (%d-%d)\n", bx, ex, b, e);
#endif

    setStrip(bx, ex);
    setOrderReferenceLine(bx);

    //Step 1: base case (unit strip)
    if (e - b == 1) {
        setOrderReferenceLine(ex);
        searchStrip(L_v, R_v);
#ifdef DEBUG
        printf("\n============\n");
        printf("vertical strip: %lf %lf - (%d-%d)\n", bx, ex, b, e);
        printf("L_v:\n");
        printoset(L_v);
        printf("R_v:\n");
        printoset(R_v);
        printf("============\n");
#endif
        return;
    }

    //Step 2:
    oset Q_v(&ls_cmp), L_ls_v(&ls_cmp);
    split(L_v, Q_v, L_ls_v);

    //Step 3:
    o_findint(Q_v, L_ls_v);

    //Step 4: compute the split index
    int c = (b + e) / 2;
    double cx = xevents[c].endpoints[0]->pt->x;

    //Step 5: rebuild the inner line segments unordered sets to prepare the recursive calls.
    uset I_ls_v(&ls_set_cmp), I_rs_v(&ls_set_cmp);
    for (uset::iterator it = I_v.begin(); it != I_v.end(); it++) {
        if (it->p2->x < cx) // if *it is inner to the interval [b, c[
            I_ls_v.insert(*it);

        if (it->p1->x > cx) // if *it is inner to the interval [c, e[
            I_rs_v.insert(*it);
    }

    //Step 6: left strip recursive call
    oset R_ls_v(&ls_cmp);
    TreeSearch(L_ls_v, I_ls_v, b, c, R_ls_v);
    setStrip(bx, ex);
    setOrderReferenceLine(cx);

    //Step 7:
    oset L_rs_v = R_ls_v;
    for (int i = 0; i < xevents[c].nevents; i++) {
        if (xevents[c].endpoints[i]->type == LEFT_ENDPOINT) {
            L_rs_v.insert(xevents[c].endpoints[i]->ls);
        } else {
            L_rs_v.erase(xevents[c].endpoints[i]->ls);
        }
    }
    setOrderReferenceLine(bx);

    //Step 8: recursive call to the right strip
    oset R_rs_v(&ls_cmp);
    TreeSearch(L_rs_v, I_rs_v, c, e, R_rs_v);
    setStrip(bx, ex);

    //Step 9:
    setOrderReferenceLine(ex);
    o_findint(Q_v, R_rs_v);

    //Step 10-11: find the intersection of Q_v and I_v, knowing that I_v is unordered.
    u_findint(Q_v, I_v);

    //Step 12:
    setOrderReferenceLine(ex);
    merge(Q_v, R_rs_v, R_v);

    //Step 13: in order to support connected line segments and find their intersections
    //we need to find the intersections between R_ls_v and L_rs_v at the sweep line x=cx
    findIntersectionsAtLine(L_rs_v, R_ls_v, cx);

#ifdef DEBUG
    printf("\n============\n");
    printf("vertical strip: %lf %lf - (%d-%d)\n", bx, ex, b, e);
    printf("split line: %lf (%d)\n", cx, c);

    setOrderReferenceLine(bx);
    printf("L_v:\n");
    printoset(L_v);
    printf("I_v:\n");
    printuset(I_v);
    setOrderReferenceLine(ex);
    printf("R_v:\n");
    printoset(R_v);

    setOrderReferenceLine(bx);
    printf("Q_v:\n");
    printoset(Q_v);

    printf("L_ls_v:\n");
    printoset(L_ls_v);
    printf("I_ls_v:\n");
    printuset(I_ls_v);
    setOrderReferenceLine(cx);
    printf("R_ls_v:\n");
    printoset(R_ls_v);

    printf("L_rs_v:\n");
    printoset(L_rs_v);
    printf("I_rs_v:\n");
    printuset(I_rs_v);
    setOrderReferenceLine(ex);
    printf("R_rs_v:\n");
    printoset(R_rs_v);

    printf("============\n");
#endif
}

void ivbalaban::findIntersectionsAtLine(oset &A, oset &B, double x) {
    setOrderReferenceLine(x);
    oset::iterator i = A.begin(), j = B.begin();

    while (i != A.end() && j != B.end()) {
        double y_i = getYatX(*i, x), y_j = getYatX(*j, x);

        if (fabs(y_i - y_j) <= 0.00001) {
            reportIntersection(*i, *j);
            i++;
            j++;
        } else {
            if (y_i < y_j)
                i++;
            else
                j++;
        }
    }
}

void ivbalaban::findIntersectionSegmentStaircase(oset &A, oset::iterator it, GIMS_LineSegment seg) {
    if (it != A.end())
        reportIntersection(*it, seg);

    oset::iterator i = oset::iterator(it);
    for (i++; i != A.end(); i++) {
        if (!reportIntersection(*i, seg))
            break;
    }

    i = oset::iterator(it);
    if (i != A.begin()) {
        for (i--;; i--) {
            if (!reportIntersection(*i, seg))
                break;

            if (i == A.begin())
                break;
        }
    }
}

void ivbalaban::o_findint(oset A, oset B) {
    if (A.empty() || B.empty())
        return;

    oset::iterator j = B.begin();

    for (oset::iterator i = A.begin(); i != A.end(); i++) {
        while (j != B.end() && getYatX(*i, sweepline_x) >= getYatX(*j, sweepline_x)) {
            findIntersectionSegmentStaircase(A, i, *j);
            j++;
        }
    }

    oset::iterator last = A.end();
    last--;
    while (j != B.end()) {
        findIntersectionSegmentStaircase(A, last, *j);
        j++;
    }
}

void ivbalaban::u_findint(oset A, uset B) {
    if (A.empty() || B.empty())
        return;

    for (uset::iterator it = B.begin(); it != B.end(); it++) {
        double prev_x = sweepline_x;

        double x = MAX(it->p1->x, vstrip[0]);
        setOrderReferenceLine(x);

        oset::iterator after = A.upper_bound(*it);
        if (after == A.end())
            after--;

        setOrderReferenceLine(prev_x);
        findIntersectionSegmentStaircase(A, after, *it);
    }
}

//finds the intersections between L and R and
//complexity: O( L + int(L) )
void ivbalaban::searchStrip(oset &L, oset &R) {
#ifdef DEBUG
    printf("===== searchStrip (%lf %lf)=====\n", vstrip[0], vstrip[1]);
    printf("L:\n");
    printoset(L);
#endif

    oset Q(&ls_cmp), Ll(&ls_cmp);
    split(L, Q, Ll); //split L into Q and Ll

#ifdef DEBUG
    printf("Q:\n");
    printoset(Q);
    printf("Ll:\n");
    printoset(Ll);
#endif

    if (Ll.empty()) {
        R = Q;
        return;
    }

    o_findint(Q, Ll); //find intersections between Q and Ll
    oset Rl(&ls_cmp);
    searchStrip(Ll, Rl); //find intersections in Ll

    merge(Q, Rl, R); //merge Q and Rl into R

#ifdef DEBUG
    printf("===== searchStrip =====\n");
#endif
}

//merges A and B into another empty oset R.
//complexity: O(A + B)
void ivbalaban::merge(oset &A, oset &B, oset &R) {
    oset::iterator i = A.begin();
    oset::iterator j = B.begin();

    while (i != A.end() || j != B.end()) {
        int c;
        if (i == A.end())
            c = 1;
        else if (j == B.end())
            c = -1;
        else
            c = ls_cmp(*i, *j);

        if (c < 0) {
            R.insert(R.end(), *i);
            i++;
        } else {
            R.insert(R.end(), *j);
            j++;
        }
    }
}

bool ivbalaban::isSpanning(const GIMS_LineSegment &l) {
    if (l.p1->x <= vstrip[0] && l.p2->x >= vstrip[1])
        return true;
    return false;
}

//splits the ordered oset L<_x into oset Q and oset Ll
//complexity: O(L)
void ivbalaban::split(oset &L, oset &Q, oset &Ll) {
    oset::iterator Q_last = Q.end();

    for (oset::iterator it = L.begin(); it != L.end(); it++) {
        if (isSpanning(*it) && (Q.empty() || !intersectsWithinStrip(*Q_last, *it))) {
            Q_last = Q.insert(Q.end(), *it);
        } else {
            Ll.insert(Ll.end(), *it);
        }
    }
}

//comparison functions
bool ivbalaban::ls_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b) {
    double y1 = getYatX(a, sweepline_x), y2 = getYatX(b, sweepline_x);
    return y1 < y2;
}
bool ivbalaban::geom_set_cmp(const GIMS_Geometry *_a, const GIMS_Geometry *_b) {
    if (_a->type == POINT && _b->type == POINT) {
        GIMS_Point *a = (GIMS_Point *)_a, *b = (GIMS_Point *)_b;
        return a->x != b->x ? a->x < b->x : a->y < b->y;

    } else if (_a->type == LINESEGMENT && _b->type == LINESEGMENT) {
        GIMS_LineSegment *a = (GIMS_LineSegment *)_a, *b = (GIMS_LineSegment *)_b;
        int c1 = pt_cmp(*(a->p1), *(b->p1));
        if (c1 != 0)
            return c1 < 0;
        int c2 = pt_cmp(*(a->p2), *(b->p2));
        return c2 < 0;

    } else {
        return _a->type < _b->type;
    }
}
int ivbalaban::pt_cmp(const GIMS_Point &a, const GIMS_Point &b) {
    if (a.x < b.x)
        return -1;
    else if (a.x == b.x)
        if (a.y < b.y)
            return -1;
        else if (a.y > b.y)
            return 1;
        else
            return 0;
    else
        return 1;
}
bool ivbalaban::ls_set_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b) {
    int c1 = pt_cmp(*(a.p1), *(b.p1)), c2 = pt_cmp(*(a.p2), *(b.p2));
    if (c1 < 0 || (c1 == 0 && c2 < 0))
        return true;
    return false;
}

//compare two endpoints for sorting.
//complexity: O(1)
int ivbalaban::endpt_cmp(const void *_a, const void *_b) {
    endpoint a = *((endpoint *)_a);
    endpoint b = *((endpoint *)_b);

    if (a.pt->x < b.pt->x)
        return -1;
    else if (a.pt->x > b.pt->x)
        return 1;
    else {
        if (a.pt->y < b.pt->y)
            return -1;
        else if (a.pt->y > b.pt->y)
            return 1;
        else
            return 0;
    }
}

//utility functions
bool ivbalaban::reportIntersection(const GIMS_LineSegment &A, const GIMS_LineSegment &B) {
    GIMS_Geometry *it = ((GIMS_LineSegment)A).intersects(&B);

    if (it == NULL)
        return false;

    bool inStrip;

    if (it->type == POINT) {
        GIMS_Point *p = (GIMS_Point *)it;
        inStrip = p->x >= vstrip[0] && p->x <= vstrip[1];

        //if the intersection is a Line Segment
    } else {
        GIMS_LineSegment *ls = (GIMS_LineSegment *)it;
        double minx = MIN(ls->p1->x, ls->p2->x), maxx = MAX(ls->p1->x, ls->p2->x);
        if (minx <= vstrip[0]) {
            inStrip = maxx >= vstrip[0];
        } else {
            inStrip = minx <= vstrip[1];
        }
    }

    if (inStrip && A.id != B.id) {
        intersections.insert(it);
    } else {
        delete it;
    }
    return inStrip;
}
bool ivbalaban::intersectsWithinStrip(const GIMS_LineSegment &A, const GIMS_LineSegment &B) {
    GIMS_Geometry *it = ((GIMS_LineSegment)A).intersects(&B);

    if (it == NULL)
        return false;

    if (it->type == POINT) {
        GIMS_Point *p = (GIMS_Point *)it;
        bool inStrip = p->x >= vstrip[0] && p->x < vstrip[1];
        delete p;
        return inStrip;

        //if the intersection is a Line Segment
    } else {
        GIMS_LineSegment *ls = (GIMS_LineSegment *)it;
        double minx = MIN(ls->p1->x, ls->p2->x), maxx = MAX(ls->p1->x, ls->p2->x);
        delete ls;
        if (minx <= vstrip[0]) {
            return maxx >= vstrip[0];
        } else {
            return minx < vstrip[1];
        }
    }
}
double ivbalaban::getYatX(const GIMS_LineSegment &ls, double x) {
    if (ls.p1->y == ls.p2->y)
        return ls.p1->y;

    double m = (ls.p2->y - ls.p1->y) / (ls.p2->x - ls.p1->x), b = ls.p1->y - m * ls.p1->x;

    return m * x + b;
}

//input perturbation
GIMS_Geometry *ivbalaban::revertSkewness(const GIMS_Geometry *g) {
    if (g->type == POINT) {
        GIMS_Point *p = (GIMS_Point *)g;
        return new GIMS_Point(p->x - skewness * p->y, p->y);
    } else {
        GIMS_LineSegment *l = (GIMS_LineSegment *)g;
        return new GIMS_LineSegment((GIMS_Point *)revertSkewness(l->p1), (GIMS_Point *)revertSkewness(l->p2));
    }
}
void ivbalaban::applySkewness(GIMS_Point &p) {
    p.x += skewness * p.y;
}

//verbose functions
void ivbalaban::dumplineseg(GIMS_LineSegment seg) {
    printf("%.4lf %.4lf <> %.4lf %.4lf", seg.p1->x, seg.p1->y, seg.p2->x, seg.p2->y);
}
void ivbalaban::printoset(oset &s) {
    for (oset::iterator it = s.begin(); it != s.end(); it++)
        printf("p1: %.4lf %.4lf, p2: %.4lf %.4lf, y at %.4lf: %.4lf\n", it->p1->x, it->p1->y, it->p2->x, it->p2->y, sweepline_x, getYatX(*it, sweepline_x));
}
void ivbalaban::printuset(uset &s) {
    for (uset::iterator it = s.begin(); it != s.end(); it++)
        printf("p1: %.4lf %.4lf, p2: %.4lf %.4lf, y at %.4lf: %.4lf\n", it->p1->x, it->p1->y, it->p2->x, it->p2->y, sweepline_x, getYatX(*it, sweepline_x));
}
