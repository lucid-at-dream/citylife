#include "GIMSGeometry.hpp"

using namespace GIMSGeometry;

/*
-->GIMSPoint
*/
bool GIMSPoint::isInsideBox ( GIMSBoundingBox *box ) {
    /*
    double ymax = box->upperRight->y + ERR_MARGIN,
           ymin = box->lowerLeft->y  - ERR_MARGIN,
           xmax = box->upperRight->x + ERR_MARGIN,
           xmin = box->lowerLeft->x  - ERR_MARGIN;
    */
    if ( this->x <= box->upperRight->x + ERR_MARGIN &&  //xmax
            this->x >= box->lowerLeft->x  - ERR_MARGIN &&  //xmin
            this->y <= box->upperRight->y + ERR_MARGIN &&  //ymax
            this->y >= box->lowerLeft->y  - ERR_MARGIN     //ymin
       ) {
        return true;
    }
    
    return false;
}

GIMSSide GIMSPoint::sideOf ( GIMSEdge *edge) {
    double s = (edge->p2->x - edge->p1->x) * (this->y - edge->p1->y) -
               (edge->p2->y - edge->p1->y) * (this->x - edge->p1->x);

    TODO (ERR_MARGIN might not be the most suited error margin for this operation)
    return s < -ERR_MARGIN ? RIGHT : s > ERR_MARGIN ? LEFT : ALIGNED;
}

GIMSPoint::GIMSPoint (double x, double y) {
    this->x = x;
    this->y = y;
    this->type = POINT;
}

GIMSPoint::~GIMSPoint() {
    //There's nothing nested to deallocate here.
}
/*
GIMSPoint<--
*/


/*
class GIMSBoundingBox : public GIMSGeometry {
  public:
    GIMSPoint *lowerLeft ,
              *upperRight;
              
    GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight );
    ~GIMSBoundingBox();
};

class GIMSEdge : public GIMSGeometry {
  public:
    GIMSPoint *p1,
              *p2;
    GIMSEdge ( GIMSPoint *p1, GIMSPoint *p2 );
    ~GIMSEdge();
};

class GIMSEdgeList  : public GIMSGeometry {
    std::list<GIMSEdge *> list;
};

class GIMSPointList  : public GIMSGeometry {
    std::list<GIMSPoint *> list;
};

class GIMSGeometryList  : public GIMSGeometry {
    std::list<GIMSGeometry *> list;
};
*/

