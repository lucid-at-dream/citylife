#ifndef GEOMETRYCOLLECTION_HPP
#define GEOMETRYCOLLECTION_HPP

#include "GIMSGeometry.hpp"

namespace GIMS_GEOMETRY{

    class GIMS_GeometryCollection  : public GIMS_Geometry {
      public:
        int size;
        int allocatedSize;
        GIMS_Geometry **list;

        void                     append                  (GIMS_Geometry &);
        GIMS_GeometryCollection *clone                   ();
        GIMS_Geometry           *clipToBox               (GIMS_BoundingBox &);
                                 GIMS_GeometryCollection (int size);
                                 GIMS_GeometryCollection ();
                                ~GIMS_GeometryCollection ();
    };

}
#endif
