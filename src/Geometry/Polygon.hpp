#ifndef POLYGON_HPP
#define POLYGON_HPP

#include "GIMSGeometry.hpp"

namespace GIMS_GEOMETRY{
    class GIMS_Polygon : public GIMS_Geometry {
      public:
        int ir_allocatedSize, ir_size;

        GIMS_LineString  *externalRing,
                        **internalRings;

        GIMS_Polygon  *clone              ();
        GIMS_Geometry *clipToBox          (GIMS_BoundingBox *);
        void           appendInternalRing (GIMS_LineString *);
                       GIMS_Polygon       (GIMS_LineString *, GIMS_LineString **);
                       GIMS_Polygon       ();
                      ~GIMS_Polygon       ();
    };
}

#endif
