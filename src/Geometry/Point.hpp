#ifndef POINT_HPP
#define POINT_HPP

#include "GIMSGeometry.hpp"

namespace GIMSGEOMETRY{
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
  };
}

#endif
