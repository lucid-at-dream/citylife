#ifndef _POLYGONINTERSECTION_HPP_
#define _POLYGONINTERSECTION_HPP_

#include "DCEL.hpp"
#include "Geometry.hpp"
#include "DE9IM.hpp"

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
void addPointToPointList(pointlist &ptlist, GIMS_Point *p);
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

#endif