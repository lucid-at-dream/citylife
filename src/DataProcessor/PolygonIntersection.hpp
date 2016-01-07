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
typedef struct _pointlist{
    int nnodes, allocated;
    GIMS_Point *nodes;
}pointlist;

typedef struct _pointmatrix{
    int nrows, allocated;
    pointlist *matrix;
}pointmatrix;

/*functions*/
pointmatrix newPointMatrix();
void addListToPointMatrix(pointmatrix &matrix, pointlist &ptlist);
pointlist newPointList();
void addPointToPointList(pointlist &ptlist, GIMS_Point *p, int data);
int __plist_cmp__(const void *_a, const void *_b);
void sortPointListWithRegardToFirst(pointlist &ptlist);
void removeRepeatedPoints(pointlist &ptlist);
void mergePointList(pointlist &keeper, pointlist &other);
DCEL polygonAndDomainAsPlanarGraph(GIMS_Polygon *P, GIMS_BoundingBox *domain);
GIMS_Polygon *clipPolygonInDCEL(DCEL planargraph);
DCEL buildPlanarGraph(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB, GIMS_BoundingBox *domain);
void DE9IM_pol_pol(DE9IM *resultset, GIMS_Polygon *A, GIMS_Polygon *B, GIMS_BoundingBox *domain);

/*global vars*/
extern GIMS_Point *__ptlist_cmp__reference__;

/*line intersection stuff*/
typedef struct _PolygonIntersectionEvent{
    GIMS_Point *pt;
    char type;
    GIMS_LineSegment ls;
}PolygonIntersectionEvent;

bool event_cmp(PolygonIntersectionEvent a, PolygonIntersectionEvent b);
bool ls_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b);

typedef list<GIMS_LineSegment> lsset;
typedef priority_queue<PolygonIntersectionEvent, vector<PolygonIntersectionEvent>,decltype(&event_cmp)> eventset;
typedef map<GIMS_LineSegment, list<GIMS_Point *>, decltype(&ls_cmp)> intersectionset;

intersectionset findIntersections(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB);
#endif