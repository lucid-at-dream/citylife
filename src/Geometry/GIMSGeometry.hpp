#ifndef GIMS_GEOMETRY_HPP
#define GIMS_GEOMETRY_HPP

#include <list>
#include "SystemBase.hpp"
#include <cmath>

namespace GIMS_GEOMETRY {

    enum GeometryType {
        //envolving quad (aka envelope)
        BOUNDINGBOX = 1<<0,
        //Elementary datatypes
        POINT       = 1<<1,
        LINESTRING  = 1<<2,
        RING        = 1<<3, //(a line string that is closed)
        POLYGON     = 1<<4,
        //Multi datatypes
        MULTIPOINT      = 1<<5,
        MULTILINESTRING = 1<<6
        MULTIPOLYGON    = 1<<7,
        //collections
        GEOMETRYCOLLECTION = 1<<8,
        //auxiliary type. In fact it is a Linestring with only two points.
        LINESEGMENT = 1<<9;
    };

    enum GIMS_Side {
        RIGHT   = 1<<0,
        LEFT    = 1<<1,
        ALIGNED = 1<<2,
    };

    class GIMS_Geometry; //base class
    class GIMS_BoundingBox;
    class GIMS_Point;
    class GIMS_LineString;
    class GIMS_Ring;
    class GIMS_Polygon;
    class GIMS_MultiPoint;
    class GIMS_MultiLineString;
    class GIMS_MultiPolygon;
    class GIMS_GeometryCollection;
    class GIMS_LineSegment;

    class GIMS_Geometry {
      public:
        unsigned int renderCount; //we need a better way of doing this.

        GeometryType type;
        unsigned long long int id; //osm_id

        //returns the subset of geometry component that intersect a given bounding box
        virtual GIMS_Geometry *clipToBox     ( GIMS_BoundingBox & ) = 0;
        
        virtual GIMS_Geometry *clone         () = 0;
        virtual               ~GIMS_Geometry ();
    };
}

using namespace GIMS_GEOMETRY;

/*Utility functions*/
double distanceSquared2p(GIMS_Point &p1, GIMS_Point &p2);
double angle3p(GIMS_Point &p1, GIMS_Point &p2, GIMS_Point &p3);
double cosine3p(GIMS_Point &p1, GIMS_Point &p2, GIMS_Point &p3);
double distToSegmentSquared(GIMS_Point &p, GIMS_Edge &e);

#include "Point.hpp"
#include "LineString.hpp"
#include "Polygon.hpp"
#include "GeometryCollection.hpp"
#include "BoundingBox.hpp"

#endif

