/* An implementation of the PM3 Quadtree.
 *
 * A PM3 Quadtree is a quadtree structure to store polygonal data (including
 * isolated vertexes and line segments) where the building criterion is as
 * follows: "At most one vertex can lie in a region represented by a quadtree
 * leaf".
 *
 * The structures highest dept is inversely proportional to the distance between
 * the two closest vertexes.
 */

#ifndef _PMQUADTREE_HPP_
#define _PMQUADTREE_HPP_

#include "ogr_geometry.h"
#include <cstdio>
#include <cstdlib>
#include <list>

#define ERR_MARGIN 0.0001

enum data_t {EDGE, POINT};
enum node_t {WHITE, GRAY, BLACK};
enum side {LEFT, ALIGNED, RIGHT};

enum quadrant {NW = 0, NE = 1, SE = 2, SW = 3};

typedef struct _point {
    double x;
    double y;
} point;

typedef struct _square {
    point *leftUpperCorner;
    double len;
} square;

typedef struct _edge {
    point *p1;
    point *p2;
    //OGRFeature *feature;
} edge;

typedef union _edgelistData {
    point *pt;
    edge *e;
} edgelistData;

typedef struct _edgelistNode {
    data_t datatype;
    edgelistData *data;
    char *color;
} edgelistNode;

typedef struct _node {
    square *s;
    struct _node *son[4];
    node_t type;
    std::list<edgelistNode *> *dictionary;
} node;


void dumpTreeJSON (node *R);
void dumpLeafJSON (node *R);
edge *trimEdge (edge *e, square *s);
void splitNode (node *R);
square *createSquare ( point *leftUpperCorner, double length );
point *createPoint (double x, double y);
node *createLeafNode (square *s);
bool validateSquare (std::list<edgelistNode *> *edges, square *S);
bool validateVertexSharing ( point *pt, std::list<edgelistNode *> *edges,
                             std::list<edgelistNode *>::iterator e, square *S );
void insert (std::list<edgelistNode *> *edges, node *R);
std::list<edgelistNode *> *clipLines (std::list<edgelistNode *> *L, square *S);
bool clipSquare (edgelistNode *e, square *S);
bool ptInsideSquare (point *pt, square *S);
edge *createEdge (point *p1, point *p2);
bool edgeInsideSquare ( edge *e, square *S );
bool lineptInsideLineSeg (point *pt, edge *e);
point *clonePoint ( point *p );
double ptEdgeSide (point *p, edge *e);

#endif
