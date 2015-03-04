#ifndef PMQUADTREE_HPP
#define PMQUADTREE_HPP

#include "DataStructs.hpp"
#include "DebRender.hpp"
#include <map>
#include <list>
#include <cmath>

using namespace std;

namespace PMQUADTREE {

    enum NodeType {WHITE, GRAY, BLACK};
    enum Quadrant {NW = 0, NE = 1, SE = 2, SW = 3};
    enum Direction {NORTH, SOUTH, EAST, WEST};

    class Node {
      public:
        NodeType type;
        GIMS_BoundingBox *square;
        list<GIMS_Geometry *> *dictionary;
        Node *father;
        Node *sons[4];

        //dictionary related functions
        list<GIMS_Geometry *> *clipDict(list<GIMS_Geometry *> *dict);

        void *search                  (GIMS_Geometry *geom);
        void *searchInterior          (GIMS_Polygon *pol);
        void  insert                  (list<GIMS_Geometry *> *);
        int   numPoints               (list<GIMS_Geometry *> *);
        bool  validate                (list<GIMS_Geometry *> *);
        bool  validateGeometry        (GIMS_Geometry *p, GIMS_Point **sharedPoint);
        bool  validatePolygon         (GIMS_Polygon *p, GIMS_Point **sharedPoint);
        bool  validatePoint           (GIMS_Point *pt, GIMS_Point **sharedPoint);
        bool  validateLineString      (GIMS_LineString *ls, GIMS_Point **sharedPoint); 
        bool  validateLineSegment     (GIMS_LineSegment *l, GIMS_Point **sharedPoint);
        bool  polygonContainsPoint    (GIMS_Polygon *pol, GIMS_Point *pt);
        GIMS_Geometry *hasReferenceTo (long id);
        Node *goNorth                 (double x);

        list<GIMS_Geometry *> *unconstrainedActiveSearch(int(*filter)(GIMS_Geometry *));
        list<GIMS_Geometry *> *activeInteriorSearch (GIMS_Polygon *pol, 
                                                    int(*intersectedFilter)(Node *, GIMS_Geometry *, GIMS_Geometry *),
                                                    int(*containedFilter)(GIMS_Geometry *) );

        void  split                   ();
              Node                    ();
              Node                    (GIMS_BoundingBox *square);
             ~Node                    ();
    };

    class PMQuadTree : public GIMS_DataStruct, public DebugRenderable {
      public:
        Node *root;
        GIMS_Geometry *query;

        /*Allocation & Deallocation*/
         PMQuadTree (GIMS_BoundingBox *domain);
        ~PMQuadTree ();

        /*Inherited Functions*/
        /*Functions that take care of the construction and maintenance of the structure*/
        void  build  (GIMS_Geometry *);
        void  insert (list<GIMS_Geometry *> *geom);
        void  insert (GIMS_Geometry *);
        void  remove (GIMS_Geometry *);
        void *search (GIMS_Geometry *);

        /*check if container contains contained. Supports only geometry primitives,
         *and not multipart geometries.*/
        bool contains(GIMS_Geometry* container, GIMS_Geometry* contained);

        /*given a topological filtering function, report all objects that relate
          to the first argument of the function.*/
        list<GIMS_Geometry *> *getRelated(GIMS_Geometry *g,
                                          int(*intersectedFilter)(Node *, GIMS_Geometry *, GIMS_Geometry *),
                                          int(*containedFilter)(GIMS_Geometry *));

        /*Follow the operations between the data structure and a given geometry*/
        RelStatus intersects_g  ( GIMS_Geometry *result, GIMS_Geometry *);
        RelStatus meets_g       ( GIMS_Geometry *result, GIMS_Geometry *);
        RelStatus contains_g    ( GIMS_Geometry *result, GIMS_Geometry *);
        RelStatus isContained_g ( GIMS_Geometry *result, GIMS_Geometry *);
        
        /*Retrieve all geometry elements that are partially or totally contained
          in a given bounding box*/
        RelStatus isBoundedBy ( GIMS_Geometry *result, GIMS_BoundingBox *);

        void renderRed      ( GIMS_Geometry * );
        void renderGreen    ( GIMS_Geometry * );
        void onClick        ( double, double );
        void debugRender    ( Cairo::RefPtr<Cairo::Context> );
        void renderTree     ( Cairo::RefPtr<Cairo::Context>, Node *n );
        void renderLeafNode ( Cairo::RefPtr<Cairo::Context>, Node *n );

        int getNumNodes();
        int getMaxDepth();

    };
}

void mergeDicts(list<GIMS_Geometry *> a, list<GIMS_Geometry *> b);

extern PMQUADTREE::Quadrant quadrantList[4];
extern char quadrantLabels[4][3];
extern double xmultiplier[4];
extern double ymultiplier[4];

#endif
