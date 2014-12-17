#ifndef GEOMETRYLIST_HPP
#define GEOMETRYLIST_HPP

#include "GIMSGeometry.hpp"

namespace GIMSGEOMETRY{
    class GIMSGeometryList  : public GIMSGeometry {
      public:
        std::list<GIMSGeometry *> *list;

        GIMSGeometryList *clone            ();
        GIMSGeometry *clipToBox        ( GIMSBoundingBox * );
                      GIMSGeometryList ();
                     ~GIMSGeometryList ();
    };
}
#endif
