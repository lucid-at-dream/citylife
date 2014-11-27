#ifndef PMQUADTREE_HPP
#define PMQUADTREE_HPP

#include "DataStructs.hpp"
#include "DebRender.hpp"
#include <list>
#include <cmath>

namespace PMQUADTREE {

    enum NodeType {WHITE, GRAY, BLACK};
    enum Quadrant {NW = 0, NE = 1, SE = 2, SW = 3};
    enum Direction {NORTH, SOUTH, EAST, WEST};

    class Node {
      public:
        NodeType type;
        GIMSBoundingBox *square;
        std::list<GIMSGeometry *> *dictionary;
        Node *father;
        Node *sons[4];

        void *search                ( GIMSGeometry *geom );
        void *searchInterior        ( GIMSPolygon *pol );
        void  insert                ( GIMSGeometry * );
        bool  validateGeometry      ( GIMSGeometry * );
        bool  polygonContainsPoint  (GIMSPolygon *pol, GIMSPoint *pt);
        GIMSGeometry *hasReferenceTo( unsigned long int id );
        Node *goNorth( double x );
        void  split                 ();
              Node                  ();
              Node                  ( GIMSBoundingBox *square );
             ~Node                  ();
    };

    class PMQuadTree : public GIMSDataStruct, public DebugRenderable {
      public:
        Node *root;
        GIMSGeometry *query;

        /*Allocation & Deallocation*/
         PMQuadTree (GIMSBoundingBox *domain);
        ~PMQuadTree (void);

        /*Inherited Functions*/
        /*Functions that take care of the construction and maintenance of the structure*/
        virtual void  build  (GIMSGeometry *);
        virtual void  insert (GIMSGeometry *);
        virtual void  remove (GIMSGeometry *);
        virtual void *search (GIMSGeometry *);

        /*Follow the operations between the data structure and a given geometry*/
        virtual RelStatus intersects_g  ( GIMSGeometry *result, GIMSGeometry *);
        virtual RelStatus meets_g       ( GIMSGeometry *result, GIMSGeometry *);
        virtual RelStatus contains_g    ( GIMSGeometry *result, GIMSGeometry *);
        virtual RelStatus isContained_g ( GIMSGeometry *result, GIMSGeometry *);
        
        /*Retrieve all geometry elements that are partially or totally contained
          in a given bounding box*/
        virtual RelStatus isBoundedBy ( GIMSGeometry *result, GIMSBoundingBox *);

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
