#ifndef GIMSGEOMETRY_HPP
#define GIMSGEOMETRY_HPP

#include <list>
#include "SystemBase.hpp"
#include <cmath>


namespace GIMSGEOMETRY {

    enum GeometryType {
        BOUNDINGBOX,
        //EDGELIST,
        //POINTLIST,
        MIXEDLIST,
        EDGE,
        POINT,
        POLYGON,
    };

    enum GIMSSide {
        RIGHT,
        LEFT,
        ALIGNED
    };

    class GIMSBoundingBox;
    class GIMSPoint;
    class GIMSEdge;
    class GIMSPolygon;
    class GIMSGeometryList;

    class GIMSGeometry {
      public:
        GeometryType type;
        unsigned int renderCount;
        TODO ( add the label or id )
        unsigned long int id;

        virtual GIMSGeometry *clipToBox    ( GIMSBoundingBox * ) = 0;
        virtual GIMSGeometry *clone        () = 0;
        virtual              ~GIMSGeometry ();
    };

    class GIMSPoint : public GIMSGeometry {
      public:
        double x, y;

        bool          equals                 ( GIMSPoint *cmp );
        GIMSPoint    *clone                  ();
        GIMSGeometry *clipToBox              ( GIMSBoundingBox * );
        bool          isInsideBox            ( GIMSBoundingBox *box );
        GIMSSide      sideOf                 ( GIMSEdge *edge);
        bool          isInsideEdgeOfSameLine ( GIMSEdge *edge );
                      GIMSPoint              ();
                      GIMSPoint              ( double x, double y );
                     ~GIMSPoint              ();

        inline bool operator == ( const GIMSPoint& cmp ) {
            if ( this->x <= cmp.x + ERR_MARGIN && this->x >= cmp.x - ERR_MARGIN &&
                 this->y <= cmp.y + ERR_MARGIN && this->y >= cmp.y - ERR_MARGIN ){
                return true;
            }
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

        GIMSBoundingBox *clone           ();
        double        xlength         ();
        double        ylength         ();
        double        minx            ();
        double        maxx            ();
        double        miny            ();
        double        maxy            ();
        GIMSGeometry *clipToBox       ( GIMSBoundingBox * );
                      GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight );
                     ~GIMSBoundingBox ();
    };
    
    class GIMSEdge : public GIMSGeometry {
      public:
        GIMSPoint *p1,
                  *p2;

        GIMSEdge *clone     ();
        GIMSGeometry *clipToBox ( GIMSBoundingBox * );
        GIMSEdge     *trimToBBox( GIMSBoundingBox * );
                      GIMSEdge  ( GIMSPoint *p1, GIMSPoint *p2 );
                     ~GIMSEdge  ();
    };
    
    class GIMSPolygon : public GIMSGeometry {
      public:
        GIMSGeometryList *externalRing,
                         *internalRings;

        GIMSPolygon  *clone     ();
        GIMSGeometry *clipToBox ( GIMSBoundingBox * );
                      GIMSPolygon  ( GIMSGeometryList *, GIMSGeometryList * );
                     ~GIMSPolygon  ();
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

        GIMSGeometryList *clone            ();
        GIMSGeometry *clipToBox        ( GIMSBoundingBox * );
                      GIMSGeometryList ();
                     ~GIMSGeometryList ();
    };

}

using namespace GIMSGEOMETRY;

double distanceSquared2p(GIMSPoint *p1, GIMSPoint *p2);
double angle3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3);
double cosine3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3);
double distToSegmentSquared(GIMSPoint *p, GIMSEdge *e);

#endif
