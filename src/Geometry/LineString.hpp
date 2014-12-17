#ifndef EDGE_HPP
#define EDGE_HPP

#include "GIMSGeometry.hpp"

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
#ifndef EDGELIST_HPP
#define EDGELIST_HPP

#include "GIMSGeometry.hpp"
#include "GeometryList.hpp"
#include "Polygon.hpp"
#include "Point.hpp"
#include "Edge.hpp"
#include "BoundingBox.hpp"

#include <cstring>

namespace GIMSGEOMETRY{
  class GIMSEdgeList : public GIMSGeometry {
    public:
      GIMSPoint **list;
      int         size;
      int         allocatedSize;

      GIMSEdgeList *clone        ();
      GIMSGeometry *clipToBox    (GIMSBoundingBox *);
      GIMSEdge      getEdge      (int index);
      void          appendPoint(GIMSPoint *p);
      void          appendPoint  (double, double);
                    GIMSEdgeList (int size);
                    GIMSEdgeList ();
                   ~GIMSEdgeList ();
  };
}

#endif
