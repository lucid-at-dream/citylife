#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "GIMSGeometry.hpp"
#include "Edge.hpp"
#include "GeometryList.hpp"
#include "Point.hpp"
#include "BoundingBox.hpp"

namespace GIMSGEOMETRY{
    class GIMSPolygon : public GIMSGeometry {
      public:
        GIMSGeometryList *externalRing,
                         *internalRings;

        GIMSPolygon  *clone     ();
        GIMSGeometry *clipToBox ( GIMSBoundingBox * );
                      GIMSPolygon  ( GIMSGeometryList *, GIMSGeometryList * );
                     ~GIMSPolygon  ();
    };
}

#endif
