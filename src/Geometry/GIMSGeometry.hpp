#ifndef GIMSGEOMETRY_HPP
#define GIMSGEOMETRY_HPP

#include <list>
#include "SystemBase.hpp"
#include <cmath>

namespace GIMSGEOMETRY {

    enum GeometryType {
        BOUNDINGBOX,
        EDGELIST,
        POINTLIST,
        MIXEDLIST,
        EDGE,
        POINT
    };

    enum GIMSSide {
        RIGHT,
        LEFT,
        ALIGNED
    };
    
    class GIMSGeometry {
      public:
        GeometryType type;
        TODO ( add the label )
        //OGRFeature *label;
    };

    class GIMSBoundingBox;
    class GIMSPoint;
    class GIMSEdge;
    class GIMSGeometryList;

    class GIMSPoint : public GIMSGeometry {
      public:
        double x, y;

        bool      isInsideBox            ( GIMSBoundingBox *box );
        GIMSSide  sideOf                 ( GIMSEdge *edge);
        bool      isInsideEdgeOfSameLine ( GIMSEdge *edge );
                  GIMSPoint              ( double x, double y );
                 ~GIMSPoint              ();

        inline bool operator == ( const GIMSPoint& cmp ) {
            if ( this->x <= cmp.x + ERR_MARGIN && this->x >= cmp.x - ERR_MARGIN &&
                 this->y <= cmp.y + ERR_MARGIN && this->y >= cmp.y - ERR_MARGIN )
                return true;
            return false;
        }
        
        inline bool operator != ( const GIMSPoint& cmp ) {
            return !operator == ( cmp );
        }
    };
    
    class GIMSBoundingBox : public GIMSGeometry {
      public:
        GIMSPoint *lowerLeft ,
                  *upperRight;
        double xlength();
        double ylength();
        double minx();
        double maxx();
        double miny();
        double maxy();
        GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight );
        ~GIMSBoundingBox();
    };
    
    class GIMSEdge : public GIMSGeometry {
      public:
        GIMSPoint *p1,
                  *p2;
        GIMSEdge ( GIMSPoint *p1, GIMSPoint *p2 );
        ~GIMSEdge();
    };
    
    // class GIMSEdgeList  : public GIMSGeometry {
    //   public:
    //     std::list<GIMSEdge *> *list;
    //     GIMSEdgeList();
    //     ~GIMSEdgeList();
    // };
    
    // class GIMSPointList  : public GIMSGeometry {
    //   public:
    //     std::list<GIMSPoint *> *list;
    //     GIMSPointList();
    //     ~GIMSPointList();
    // };
    
    class GIMSGeometryList  : public GIMSGeometry {
      public:
        std::list<GIMSGeometry *> *list;
        GIMSGeometryList();
        ~GIMSGeometryList();
    };
    
}

#endif
