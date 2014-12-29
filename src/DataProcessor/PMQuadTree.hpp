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

    class NodeDictionary {
      public:
        map<long, GIMS_Geometry *> *pointless,
                                   *pointful;
        GIMS_Point *shpt;
        
        bool insert(GIMS_Geometry *);

    }

    class Node {
      public:
        NodeType type;
        GIMS_BoundingBox *square;
        map<long, GIMS_Geometry *> *dictionary;
        Node *father;
        Node *sons[4];

        void *search                 (GIMS_Geometry *geom);
        void *searchInterior         (GIMS_Polygon *pol);
        void  insert                 (GIMS_Geometry *);
        bool  validateGeometry       (GIMS_Geometry *);
        bool  polygonContainsPoint   (GIMS_Polygon *pol, GIMS_Point *pt);
        GIMS_Geometry *hasReferenceTo (unsigned long int id);
        Node *goNorth                (double x);
        void  split                  ();
              Node                   ();
              Node                   (GIMS_BoundingBox *square);
             ~Node                   ();
    };

    class PMQuadTree : public GIMS_DataStruct, public DebugRenderable {
      public:
        Node *root;
        GIMS_Geometry *query;

        /*Allocation & Deallocation*/
         PMQuadTree (GIMS_BoundingBox *domain);
        ~PMQuadTree (void);

        /*Inherited Functions*/
        /*Functions that take care of the construction and maintenance of the structure*/
        virtual void  build  (GIMS_Geometry *);
        virtual void  insert (GIMS_Geometry *);
        virtual void  remove (GIMS_Geometry *);
        virtual void *search (GIMS_Geometry *);

        /*Follow the operations between the data structure and a given geometry*/
        virtual RelStatus intersects_g  ( GIMS_Geometry *result, GIMS_Geometry *);
        virtual RelStatus meets_g       ( GIMS_Geometry *result, GIMS_Geometry *);
        virtual RelStatus contains_g    ( GIMS_Geometry *result, GIMS_Geometry *);
        virtual RelStatus isContained_g ( GIMS_Geometry *result, GIMS_Geometry *);
        
        /*Retrieve all geometry elements that are partially or totally contained
          in a given bounding box*/
        virtual RelStatus isBoundedBy ( GIMS_Geometry *result, GIMS_BoundingBox *);

        void onClick        ( double, double );
        void debugRender    ( Cairo::RefPtr<Cairo::Context> );
        void renderTree     ( Cairo::RefPtr<Cairo::Context>, Node *n );
        void renderLeafNode ( Cairo::RefPtr<Cairo::Context>, Node *n );

    };

}

extern PMQUADTREE::Quadrant quadrantList[4];
extern char quadrantLabels[4][3];
extern double xmultiplier[4];
extern double ymultiplier[4];

#endif
