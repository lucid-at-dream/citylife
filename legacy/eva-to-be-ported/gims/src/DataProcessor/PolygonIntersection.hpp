#ifndef _POLYGONINTERSECTION_HPP_
#define _POLYGONINTERSECTION_HPP_

#include "SystemBase.hpp"
#include "DCEL.hpp"
#include "Geometry.hpp"
#include "DE9IM.hpp"
#include <set>
#include <map>
#include <iostream>
#include <list>
#include <vector>
#include <queue>

using namespace std;

/*structs*/
typedef struct _pointlist {
    int nnodes, allocated;
    GIMS_Point **nodes;
} pointlist;

typedef struct _pointmatrix {
    int nrows, allocated;
    pointlist **matrix;
} pointmatrix;

/*global vars*/
extern GIMS_Point *__ptlist_cmp__reference__;

/*line intersection stuff*/
typedef struct _PolygonIntersectionEvent {
    GIMS_Point *pt;
    char type;
    GIMS_LineSegment ls;
} PolygonIntersectionEvent;

bool event_cmp(PolygonIntersectionEvent a, PolygonIntersectionEvent b);
bool ls_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b);

typedef list<GIMS_LineSegment> lsset;
typedef priority_queue<PolygonIntersectionEvent, vector<PolygonIntersectionEvent>, decltype(&event_cmp)> eventset;
typedef map<GIMS_LineSegment, list<GIMS_Point *>, decltype(&ls_cmp)> intersectionset;

/*functions*/
DCEL polygonAndDomainAsPlanarGraph(GIMS_Polygon *P, GIMS_BoundingBox *domain);
GIMS_Polygon *clipPolygonInDCEL(DCEL planargraph);
void insertPolygonVertexes(DCEL &dcel, GIMS_Polygon *p, int id);
void insertVertexesFromList(DCEL &dcel, list<GIMS_Point *> &ptlist, int id);
void insertPolygonHalfedges(DCEL &dcel, intersectionset &iset);
void connectHalfedges(DCEL &dcel);
void DE9IM_pol_pol(DE9IM *resultset, GIMS_Polygon *A, GIMS_Polygon *B, GIMS_BoundingBox *domain);
DCEL buildPlanarGraph(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB);
void sortIntersections(intersectionset &iset);
void insertToActiveSet(lsset &l, GIMS_LineSegment &ls);
void addIntersection(intersectionset &iset, GIMS_LineSegment &A, GIMS_LineSegment &B, GIMS_Geometry *intersection);
double eventQueueFromMultiLineString(eventset &eventQueue, GIMS_MultiLineString *mls, int id);
void addEndpointsToIntersectionSet(intersectionset &iset, GIMS_MultiLineString *mls, int id);
void printData(int d);
void deleteAllExceptPoints(GIMS_MultiLineString *mls);
void deleteAllExceptPoints(GIMS_Polygon *pol);
void clearSearchByproducts();

extern list<GIMS_Point *> __deleteAfterSearch__;

#endif