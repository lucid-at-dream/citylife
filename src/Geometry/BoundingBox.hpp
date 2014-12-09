#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include "GIMSGeometry.hpp"
#include "Edge.hpp"
#include "GeometryList.hpp"
#include "Polygon.hpp"
#include "Point.hpp"

namespace GIMSGEOMETRY{
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
}

#endif
