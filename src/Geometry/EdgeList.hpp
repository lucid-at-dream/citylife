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
      GIMSPoint *list;
      int        size;
      int        allocatedSize;

      GIMSEdgeList *clone        ();
      GIMSGeometry *clipToBox    (GIMSBoundingBox *);
      GIMSEdge      getEdge      (int index);
      void          appendPoint  (double, double);
                    GIMSEdgeList (int size);
                    GIMSEdgeList ();
                   ~GIMSEdgeList ();
  };
}

#endif
