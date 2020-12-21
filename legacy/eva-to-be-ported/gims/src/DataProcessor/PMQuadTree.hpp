#ifndef PMQUADTREE_HPP
#define PMQUADTREE_HPP

#include "SystemBase.hpp"
#include "Geometry.hpp"
#include "DebRender.hpp"
#include "DE9IM.hpp"
#include "BentleySolver.hpp"
#include "conf.hpp"
#include "DCEL.hpp"
#include "LineIntersection.hpp"
#include "PolygonIntersection.hpp"
#include "LinePolygonIntersection.hpp"
#include "tunasolver.hpp"

#include <list>
#include <cmath>

using namespace std;

namespace PMQUADTREE
{
enum NodeType { WHITE, GRAY, BLACK };
enum Quadrant { NW = 0, NE = 1, SE = 2, SW = 3 };
enum Direction { NORTH, SOUTH, EAST, WEST };

class Node {
public:
    NodeType type; //wheter it is a middle node (GRAY), empty leaf node (WHITE) or non-empty leaf node (BLACK)
    GIMS_BoundingBox *square; //the node's bounding square
    list<GIMS_Geometry *> *dictionary; //the node's list of geometries
    Node *father; //the node's parent
    Node *sons[4]; //the node's sons. one for each quadrant.

    /*Helper functions*/
    GIMS_Geometry *hasReferenceTo(long long id);
    Node *goNorth(double x);
    void split();

    /*dictionary related functions*/
    list<GIMS_Geometry *> *clipDict(list<GIMS_Geometry *> *dict);
    /*dictionary validation functions*/
    int numPoints(list<GIMS_Geometry *> *);
    bool validate(list<GIMS_Geometry *> *);
    bool validateGeometry(GIMS_Geometry *p, GIMS_Point **sharedPoint);
    bool validatePolygon(GIMS_Polygon *p, GIMS_Point **sharedPoint);
    bool validatePoint(GIMS_Point *pt, GIMS_Point **sharedPoint);
    bool validateLineString(GIMS_LineString *ls, GIMS_Point **sharedPoint);
    bool validateLineSegment(GIMS_LineSegment *l, GIMS_Point **sharedPoint);
    char indexedPolygonContainsPoint(GIMS_Polygon *pol, GIMS_Point *pt);

    /*Node searching functions*/
    void *search(GIMS_Geometry *geom);
    void *searchInterior(GIMS_Polygon *pol);

    /*Data manipulation functions*/
    void insert(list<GIMS_Geometry *> *);
    void remove(GIMS_Geometry *);

    /*Geometry searching functions*/
    void unconstrainedActiveSearch(DE9IM *resultset, int (*filter)(GIMS_Geometry *));
    void activeInteriorSearch(DE9IM *resultset, GIMS_Polygon *query, int (*filter)(GIMS_Geometry *));
    void activeSearch(DE9IM *resultset, GIMS_Geometry *query, int (*filter)(GIMS_Geometry *));

    /*DE9IM matrix construction functions*/
    void buildIM(DE9IM *resultset, GIMS_Geometry *query, GIMS_Geometry *other);
    void buildIM_point(DE9IM *resultset, GIMS_Point *query, GIMS_Geometry *other);
    void buildIM_linestring(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_Geometry *other);
    void buildIM_polygon(DE9IM *resultset, GIMS_Polygon *query, GIMS_Geometry *other);

    /*Node construction and destruction*/
    Node();
    Node(GIMS_BoundingBox *square);
    ~Node();
};

class PMQuadTree : public spatialIndex {
public:
    Node *root;
    GIMS_Geometry *query;

    /*Allocation & Deallocation*/
    PMQuadTree(GIMS_BoundingBox *domain);
    virtual ~PMQuadTree();

    /*Inherited Functions*/
    /*Functions that take care of the construction and maintenance of the structure*/
    void insert(list<GIMS_Geometry *> *geom);
    void insert(list<GIMS_Geometry *> &geom);
    void insert(GIMS_Geometry *);
    void remove(GIMS_Geometry *);
    void *search(GIMS_Geometry *);

    /*Find all geometries in the data structure that may relate to the given 
          geometry and for which the given function holds true*/
    DE9IM *topologicalSearch(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *));

    /*debugging functions*/
    void renderRed(GIMS_Geometry *);
    void renderGreen(GIMS_Geometry *);
    void renderBlack(GIMS_Geometry *);
    void onClick(double, double);
    void debugRender(Cairo::RefPtr<Cairo::Context>);
    void renderTree(Cairo::RefPtr<Cairo::Context>, Node *n);
    void renderLeafNode(Cairo::RefPtr<Cairo::Context>, Node *n);

    /*statistics functions*/
    int getNumNodes();
    int getMaxDepth();
};
} // namespace PMQUADTREE

DCEL polygonAndDomainAsPlanarGraph(GIMS_Polygon *P, GIMS_BoundingBox *domain);
GIMS_Polygon *clipPolygonInDCEL(DCEL planargraph);
DCEL buildPlanarGraph(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB, GIMS_BoundingBox *domain);

extern PMQUADTREE::Quadrant quadrantList[4];
extern char quadrantLabels[4][3];
extern double xmultiplier[4];
extern double ymultiplier[4];
extern unsigned long long pointcount;

extern bool __PMQT__verifyPolygonContainment__;

#endif
