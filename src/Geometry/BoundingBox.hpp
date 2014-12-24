#ifndef BOUNDINGBOX_HPP
#define BOUNDINGBOX_HPP

#include "GIMSGeometry.hpp"

namespace GIMS_GEOMETRY{
  class GIMS_BoundingBox : public GIMS_Geometry {
    public:
      GIMS_Point *lowerLeft ,
                 *upperRight;

      GIMS_BoundingBox *clone            ();
      double            xlength          ();
      double            ylength          ();
      double            minx             ();
      double            maxx             ();
      double            miny             ();
      double            maxy             ();
      GIMS_Geometry    *clipToBox        (GIMS_BoundingBox &);
                        GIMS_BoundingBox (GIMS_Point &lowerLeft, GIMS_Point &upperRight);
                       ~GIMS_BoundingBox ();
  };
}

#endif
