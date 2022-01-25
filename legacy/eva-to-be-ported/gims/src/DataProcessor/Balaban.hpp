#ifndef _BALABAN_HPP_
#define _BALABAN_HPP_

#include <set>
#include <list>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "Geometry.hpp"

using namespace std;

#define LEFT_ENDPOINT 1
#define RIGHT_ENDPOINT 2

#define MAX_N 10000

namespace ivbalaban {
typedef struct {
    int type;
    GIMS_Point *pt;
    GIMS_LineSegment ls;
} endpoint;

typedef struct {
    int nevents;
    endpoint *endpoints[100];
} xevent;

//comparison functions
bool ls_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b);
bool geom_set_cmp(const GIMS_Geometry *a, const GIMS_Geometry *b);
int pt_cmp(const GIMS_Point &a, const GIMS_Point &b);
bool ls_set_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b);
int endpt_cmp(const void *_a, const void *_b);

//set types: ordered set, unordered set, intersection set.
typedef multiset<GIMS_LineSegment, decltype(&ls_cmp)> oset;
typedef multiset<GIMS_LineSegment, decltype(&ls_set_cmp)> uset;
typedef set<GIMS_Geometry *, decltype(&geom_set_cmp)> iset;

//utility functions
bool reportIntersection(const GIMS_LineSegment &A, const GIMS_LineSegment &B);
bool intersectsWithinStrip(const GIMS_LineSegment &A, const GIMS_LineSegment &B);
double getYatX(const GIMS_LineSegment &ls, double x);

//input perturbation functions
GIMS_Geometry *revertSkewness(const GIMS_Geometry *p);
void applySkewness(GIMS_Point &p);

//verbose helper functions
void dumplineseg(GIMS_LineSegment seg);
void printoset(oset &s);
void printuset(uset &s);

//Balaban's algorithm specific functions
void TreeSearch(oset &L_v, uset &I_v, int b, int e, oset &R_v);
void findIntersectionsAtLine(oset &A, oset &B, double x);
void findIntersectionSegmentStaircase(oset &A, oset::iterator it, GIMS_LineSegment seg);
void o_findint(oset A, oset B);
void u_findint(oset A, uset B);
void searchStrip(oset &L, oset &R);
void merge(oset &A, oset &B, oset &R);
bool isSpanning(const GIMS_LineSegment &l);
void split(oset &L, oset &Q, oset &Ll);

//Algorithm helper functions
void setOrderReferenceLine(double x);
void setStrip(double b, double e);

//The only function that should ever be called from outside Balaban.* files:
list<GIMS_Geometry *> balaban(GIMS_MultiLineString *, GIMS_MultiLineString *);
} // namespace ivbalaban

#endif