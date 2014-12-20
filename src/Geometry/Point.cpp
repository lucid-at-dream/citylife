#include "Point.hpp"

/* Returns true if the point lies inside the given bounding box */
GIMS_Point *GIMS_Point::clone() {
    return new GIMS_Point(this->x, this->y);
}

/*Returns true if the point lies inside the parameter bounding box*/
bool GIMS_Point::isInsideBox ( GIMS_BoundingBox *box ) {
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
   lies. The sides are defined in the enum GIMS_Side */
GIMS_Side GIMS_Point::sideOf ( GIMS_Edge *edge) {
    double s = (edge->p2->x - edge->p1->x) * (this->y - edge->p1->y) -
               (edge->p2->y - edge->p1->y) * (this->x - edge->p1->x);
    return s < 0 ? RIGHT : (s > 0 ? LEFT : ALIGNED);
}

/*if the point lies in the interior of the bounding box, returns the point, else null*/
GIMS_Geometry *GIMS_Point::clipToBox ( GIMS_BoundingBox *box ){
    if( this->isInsideBox(box) )
        return this;
    else
        return NULL;
}

/*returns true if the coordinates of both points are the same. false otherwise.*/
bool GIMS_Point::equals ( GIMS_Point *cmp ) {
    if( this->x == cmp->x && this->y == cmp->y ){
        return true;
    }
    return false;
}

GIMS_Point::GIMS_Point() {
    this->type = POINT;
}

GIMS_Point::GIMS_Point (double x, double y) {
    this->x = x;
    this->y = y;
    this->type = POINT;
}

GIMS_Point::~GIMS_Point() {}



/*Multi point implementation below*/
void GIMS_MultiPoint::append(GIMS_Point *pt){
    this->size += 1;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_Point **)realloc(this->size * sizeof(GIMS_Point *));
        this->allocatedSize = this->size;
    }
    this->list[this->size-1] = pt;
}

GIMS_MultiPoint *GIMS_MultiPoint::clone(){
    GIMS_MultiPoint *fresh = new GIMS_MultiPoint(this->allocatedSize);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_Point *));
    fresh->size = this->size;
    return fresh;
}

GIMS_Geometry *GIMS_MultiPoint::clipToBox(GIMS_BoundingBox *box){
    MultiPoint *clipped = null;
    for(int i=0; i<this->size; i++){
        if( this->list[i]->isInsideBox(box) ){
            if(clipped == NULL)
                clipped = new GIMS_MultiPoint(1);
            clipped->append(this->list[i]);
        }
    }
    return clipped;
}

GIMS_MultiPoint::GIMS_MultiPoint(int size){
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
}

GIMS_MultiPoint::GIMS_MultiPoint(){
    this->size = this->allocatedSize = 0;
    this->list = NULL;
}

GIMS_MutliPoint::~GIMS_MultiPoint(){
    free(this->list);
}

