#ifndef EDGE_HPP
#define EDGE_HPP

#include "GIMSGeometry.hpp"
#include "GeometryList.hpp"
#include "Polygon.hpp"
#include "Point.hpp"
#include "BoundingBox.hpp"

namespace GIMSGEOMETRY{
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
}

#endif
