#ifndef GIMSGEOMETRY_HPP
#define GIMSGEOMETRY_HPP

#include <list>
#include "SystemBase.hpp"

namespace GIMSGeometry {

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
    class GIMSEdge;

    class GIMSPoint : public GIMSGeometry {
      public:
        double x, y;

        bool      isInsideBox            ( GIMSBoundingBox *box );
        GIMSSide  sideOf                 ( GIMSEdge *edge);
        bool      isInsideEdgeOfSameLine ( GIMSEdge *edge );
                  GIMSPoint              ( double x, double y );
                 ~GIMSPoint              ();
    };
    
    class GIMSBoundingBox : public GIMSGeometry {
      public:
        GIMSPoint *lowerLeft ,
                  *upperRight;
                  
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
    
    class GIMSEdgeList  : public GIMSGeometry {
        std::list<GIMSEdge *> list;
    };
    
    class GIMSPointList  : public GIMSGeometry {
        std::list<GIMSPoint *> list;
    };
    
    class GIMSGeometryList  : public GIMSGeometry {
        std::list<GIMSGeometry *> list;
    };
    
};

#endif
