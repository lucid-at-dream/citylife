#ifndef PMQUADTREE_HPP
#define PMQUADTREE_HPP

#include "DataStructs.hpp"
#include "DebRender.hpp"
#include <list>

namespace PMQUADTREE {

    enum NodeType {WHITE, GRAY, BLACK};
    enum Quadrant {NW = 0, NE = 1, SE = 2, SW = 3};

    class Node {
      public:
        NodeType type;
        GIMSBoundingBox *square;
        std::list<GIMSGeometry *> *dictionary;
        Node *sons[4];

        void *search                ( GIMSGeometry *geom );
        void  insert                ( GIMSGeometry * );
        bool  validateGeometry      ( GIMSGeometry * );
        bool  validateVertexSharing ( GIMSPoint *pt, 
                                      std::list<GIMSGeometry *> *, 
                                      std::list<GIMSGeometry *>::iterator );
        void  split                 ();
              Node                  ();
              Node                  ( GIMSBoundingBox *square );
             ~Node                  ();
    };

    class PMQuadTree : public GIMSDataStruct, public DebugRenderable {
      public:
        Node *root;

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

#endif
