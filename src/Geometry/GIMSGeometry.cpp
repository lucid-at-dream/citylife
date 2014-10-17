#include "GIMSGeometry.hpp"

using namespace GIMSGEOMETRY;

/*
-->GIMSGeometry
*/
GIMSGeometry::~GIMSGeometry (){}
/*
GIMSGeometry<--
*/



/*
-->GIMSPoint
*/
/* Returns true if the point lies inside the given bounding box */
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

/* Returns to which side of the vector defined by (edge->p1, edge->p2) the point
   lies. The sides are defined in the enum GIMSSide */
GIMSSide GIMSPoint::sideOf ( GIMSEdge *edge) {
    double s = (edge->p2->x - edge->p1->x) * (this->y - edge->p1->y) -
               (edge->p2->y - edge->p1->y) * (this->x - edge->p1->x);

    TODO (raw ERR_MARGIN might not be the most suited error margin for this operation)
    return s < -ERR_MARGIN ? RIGHT : s > ERR_MARGIN ? LEFT : ALIGNED;
}

/* Assuming that this point lies on the line defined by the line segment "edge",
   this function returns true if the point lies on the given line segment. */
bool GIMSPoint::isInsideEdgeOfSameLine( GIMSEdge *edge ){
    double maxx, minx;
    if( edge->p1->x > edge->p2->x ){
        maxx = edge->p1->x;
        minx = edge->p2->x;
    }else{
        maxx = edge->p2->x;
        minx = edge->p1->x;
    }

    return this->x <= maxx + ERR_MARGIN && this->x >= minx - ERR_MARGIN;
}

GIMSGeometry *GIMSPoint::clipToBox ( GIMSBoundingBox *box ){
    if( this->isInsideBox(box) )
        return this;
    else
        return NULL;
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
-->GIMSBoundingBox
*/
double GIMSBoundingBox::xlength(){
    return fabs( this->upperRight->x - this->lowerLeft->x );
}

double GIMSBoundingBox::ylength(){
    return fabs( this->upperRight->y - this->lowerLeft->y );
}

double GIMSBoundingBox::minx(){
    return this->lowerLeft->x;
}

double GIMSBoundingBox::maxx(){
    return this->upperRight->y;
}

double GIMSBoundingBox::miny(){
    return this->lowerLeft->y;
}

double GIMSBoundingBox::maxy(){
    return this->upperRight->y;
}

/*Unsupported*/
GIMSGeometry *GIMSBoundingBox::clipToBox ( GIMSBoundingBox * ){
    fprintf(stderr, "Called clipToBox on a bounding box, which is not supported.");
    exit(1);
}

GIMSBoundingBox::GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight ) {
    this->lowerLeft = lowerLeft;
    this->upperRight = upperRight;
    this->type = BOUNDINGBOX;
}

GIMSBoundingBox::~GIMSBoundingBox(){
    delete this->lowerLeft;
    delete this->upperRight;
}
/*
GIMSBoundingBox<--
*/



/*
-->GIMSEdge
*/
GIMSEdge::GIMSEdge ( GIMSPoint *p1, GIMSPoint *p2 ){
    this->p1 = p1;
    this->p2 = p2;
}

GIMSEdge::~GIMSEdge() {
    delete this->p1;
    delete this->p2;
}

GIMSGeometry *GIMSEdge::clipToBox ( GIMSBoundingBox *box ){
    
    //lowerLeft  = box->lowerLeft
    //upperRight = box->upperRight

    GIMSPoint upperLeft  = { box->lowerLeft->x , box->upperRight->y },
              lowerRight = { box->upperRight->x, box->lowerLeft->y  };

    GIMSPoint *squarePoints[] = {&upperLeft, box->upperRight, &lowerRight, box->lowerLeft};
    
    /*If all square's points lie on the same side of the line defined by the
      edge's two endpoints, then there's no intersection*/
    bool allOnSameSide = true;
    double prev = squarePoints[0]->sideOf(this);
    for ( int i = 1; i < 4; i++ ) {
        if( squarePoints[i]->sideOf(this) != prev ){
            allOnSameSide = false;
            break;
        }
    }
    if (allOnSameSide) {
        return NULL;
    }
    
    /*this part is reached only if the line defined by the two edge's endpoints
      intersects the square. Therefore, we do a projection on both the x and y
      axis of both the edge and the square and check for overlapings*/
    if ( this->p1->x > lowerRight.x &&
         this->p2->x > lowerRight.x    ) {
        /*edge is to the right of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->x < upperLeft.x &&
         this->p2->x < upperLeft.x     ) {
        /*edge is to the left of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->y > upperLeft.y && 
         this->p2->y > upperLeft.y    ) {
        /*edge is above of the rectangle*/
        return NULL;
    }
    
    if ( this->p1->y < lowerRight.y && 
         this->p2->y < lowerRight.y    ) {
        /*edge is above of the rectangle*/
        return NULL;
    }
    
    return this;
}
/*
GIMSEdge<--
*/



/*
-->GIMSEdgeList
*/
// GIMSEdgeList::GIMSEdgeList(){
//     this->list = new std::list<GIMSEdge *>();
// }

// GIMSEdgeList::~GIMSEdgeList(){
//     delete this->list;
// }
/*
GIMSEdgeList<--
*/



/*
-->GIMSPointList
*/
// GIMSPointList::GIMSPointList(){
//     this->list = new std::list<GIMSPoint *>();
// }

// GIMSPointList::~GIMSPointList(){
//     delete this->list;
// }
/*
GIMSPointList<--
*/



/*
-->GIMSGeometryList
*/
GIMSGeometry *GIMSGeometryList::clipToBox ( GIMSBoundingBox *box ){
    GIMSGeometryList *clipped = NULL;

    for( std::list<GIMSGeometry *>::iterator it = this->list->begin();
         it != this->list->end(); it++                                 ) {
        GIMSGeometry *g = (*it)->clipToBox(box);

        if( g != NULL ){
            if( clipped == NULL ){
                clipped = new GIMSGeometryList;
            }
            clipped->list->push_back(g);
        }
    }
    return clipped;
}

GIMSGeometryList::GIMSGeometryList(){
    this->list = new std::list<GIMSGeometry *>();
}

GIMSGeometryList::~GIMSGeometryList(){
    delete this->list;
}
/*
GIMSGeometryList<--
*/
