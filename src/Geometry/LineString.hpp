#ifndef EDGE_HPP
#define EDGE_HPP

#include "GIMSGeometry.hpp"
#include <cstring>

namespace GIMS_GEOMETRY{
    class GIMS_LineSegment : public GIMS_Geometry {
      public:
        GIMS_Point *p1,
                   *p2;

        GIMS_LineSegment *clone            ();
        GIMS_Geometry    *clipToBox        (GIMS_BoundingBox &);
        GIMS_LineSegment *trimToBBox       (GIMS_BoundingBox &);
                          GIMS_LineSegment (GIMS_Point &p1, GIMS_Point &p2);
                         ~GIMS_LineSegment ();
    };

    class GIMS_LineString : public GIMS_Geometry {
      public:
        GIMS_Point **list;
        int          size;
        int          allocatedSize;

        GIMS_LineString  *clone           ();
        GIMS_Geometry    *clipToBox       (GIMS_BoundingBox &);
        GIMS_LineSegment  getLineSegment  (int index);
        void              appendPoint     (GIMS_Point &p);
                          GIMS_LineString (int size);
                          GIMS_LineString ();
                         ~GIMS_LineString ();
    };

    class GIMS_Ring : public GIMS_LineString{
      public:
         GIMS_Ring(int size);
         GIMS_Ring();
        ~GIMS_Ring();
    };

    class GIMS_MultiLineString : public GIMS_Geometry {
      public:
        GIMS_LineString **list;
        int               size;
        int               allocatedSize;

        GIMS_MultiLineString *clone                ();
        GIMS_Geometry        *clipToBox            (GIMS_BoundingBox &);
        void                  append               (GIMS_Point &p);
                              GIMS_MultiLineString (int size);
                              GIMS_MultiLineString ();
                             ~GIMS_MultiLineString ();
    };
}

#endif
