#ifndef POINT_HPP
#define POINT_HPP

#include "GIMSGeometry.hpp"

namespace GIMS_GEOMETRY{
    class GIMS_Point : public GIMS_Geometry {
      public:
        double x, y;

        bool           equals     (GIMS_Point *cmp);
        GIMS_Point    *clone      ();
        GIMS_Geometry *clipToBox  (GIMS_BoundingBox *);
        bool           isInsideBox(GIMS_BoundingBox *box);
        GIMS_Side      sideOf     (GIMS_LineSegment *edge);
                       GIMS_Point ();
                       GIMS_Point (double x, double y);
                      ~GIMS_Point ();
    };

    class GIMS_MultiPoint : public GIMS_Geometry {
      public:
          int size;
          int allocatedSize;
          GIMS_Point **list;

          void             append(GIMS_Point *cmp);
          GIMS_MultiPoint *clone();
          GIMS_Geometry   *clipToBox(GIMS_BoundingBox *);
                           GIMS_MultiPoint();
                           GIMS_MultiPoint(int size);
                          ~GIMS_MultiPoint();
    };
}

#endif
