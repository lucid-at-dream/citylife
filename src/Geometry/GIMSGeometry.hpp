#ifndef GIMSGEOMETRY_HPP
#define GIMSGEOMETRY_HPP

#include <list>
#include "SystemBase.hpp"
#include <cmath>

namespace GIMSGEOMETRY {

    enum GeometryType {
        BOUNDINGBOX = 1<<0,
        EDGE        = 1<<1,
        EDGELIST    = 1<<2,
        MIXEDLIST   = 1<<3,
        POINT       = 1<<4,
        POLYGON     = 1<<5,
        RING        = 1<<6,
    };

    enum GIMSSide {
        RIGHT   = 1<<0,
        LEFT    = 1<<1,
        ALIGNED = 1<<2,
    };

    class GIMSBoundingBox;
    class GIMSPoint;
    class GIMSEdge;
    class GIMSPolygon;
    class GIMSGeometryList;

    class GIMSGeometry {
      public:
        unsigned int renderCount; //we need a better way of doing this.

        GeometryType type;
        unsigned long long int id; //osm_id

        virtual GIMSGeometry *clipToBox    ( GIMSBoundingBox * ) = 0;
        virtual GIMSGeometry *clone        () = 0;
        virtual              ~GIMSGeometry ();
    };

}

using namespace GIMSGEOMETRY;

double distanceSquared2p(GIMSPoint *p1, GIMSPoint *p2);
double angle3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3);
double cosine3p(GIMSPoint *p1, GIMSPoint *p2, GIMSPoint *p3);
double distToSegmentSquared(GIMSPoint *p, GIMSEdge *e);

#include "Edge.hpp"
#include "GeometryList.hpp"
#include "Polygon.hpp"
#include "Point.hpp"
#include "BoundingBox.hpp"

#endif

