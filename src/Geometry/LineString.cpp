#include "Geometry.hpp"

GIMS_LineSegment *GIMS_LineSegment::clone () {
    return new GIMS_LineSegment( this->p1->clone(), this->p2->clone() );
}

GIMS_LineSegment::GIMS_LineSegment ( GIMS_Point *p1, GIMS_Point *p2 ){
    this->type = LINESEGMENT;
    this->p1 = p1;
    this->p2 = p2;
}

GIMS_LineSegment::~GIMS_LineSegment() {
    delete this->p1;
    delete this->p2;
}

GIMS_Geometry *GIMS_LineSegment::clipToBox ( GIMS_BoundingBox *box ){

    GIMS_Point upperLeft  = { box->lowerLeft->x , box->upperRight->y },
               lowerRight = { box->upperRight->x, box->lowerLeft->y  };

    GIMS_Point *squarePoints[] = {&upperLeft, box->upperRight, &lowerRight, box->lowerLeft};
    
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

    /*this part is reached only if the line defined by the two linesegment's endpoints
      intersects the square. Therefore, we do a projection on both the x and y axis 
      of both the edge and the square and check for overlapings*/
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







/* From here we define the LineString class */

GIMS_LineString::GIMS_LineString (int size){
    this->type = LINESTRING;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
    this->allocatedSize = size;
    this->size = 0;
}

GIMS_LineString::GIMS_LineString (){
    this->type = LINESTRING;
    this->list = NULL;
    this->allocatedSize = 0;
    this->size = 0;
}

GIMS_LineString::~GIMS_LineString (){
    if(this->list != NULL)
        free(this->list);
}

GIMS_LineString *GIMS_LineString::clone (){
    GIMS_LineString *newList = new GIMS_LineString(this->size);
    memcpy(newList->list, this->list, this->size*sizeof(GIMS_Point *));
    return newList;
}

GIMS_Geometry *GIMS_LineString::clipToBox (GIMS_BoundingBox *box){
    GIMSMultiLineString *clipped = NULL;
    GIMS_LineString *partial = NULL;

    //if this is a ring, then the first and last points are also connected.
    int goLast = 1;
    if(this->type == RING)
        goLast = 0;

    for( int i = 0; i<this->size-goLast; i++ ){
        GIMS_LineSegment segment = this->getLineSegment(i);
        if( segment.clipToBox(box) != NULL ){
            if(partial == NULL){
                partial = new GIMS_LineString(2);
                partial->appendPoint(segment.p1);
            }
            partial->appendPoint(e.p2);
        }else if(partial != NULL){
            if(clipped == NULL)
                clipped = new GIMS_MultiLineString();
            clipped->list->push_back(el);
            partial = NULL;
        }
    }

    return clipped;
}

GIMS_LineSegment GIMS_LineString::getLineSegment (int index){
    return GIMSEdge( this->list[index%(this->size)],
                     this->list[(index+1)%(this->size)]);
}

void GIMS_LineString::appendPoint(GIMS_Point *p){
    this->size += 1;

    if( this->size > this->allocatedSize ){
        this->list = (GIMS_Point **)realloc(this->list, sizeof(GIMS_Point *) * (this->size));
        this->allocatedSize = this->size;
    }

    this->list[this->size-1] = p;
}







/*The ring class. We can take advantage of everything that was defined for LineString*/

GIMS_Ring::GIMS_Ring(int size){
    this->type = RING;
    this->size = this->allocatedSize = size;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
}

GIMS_Ring::GIMS_Ring(){
    this->type = RING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
}

GIMS_RING::~GIMS_Ring(){
    free(this->list);
}






/*The MultiLineString class.*/

GIMS_MultiLineString *GIMS_MultiLineString::clone(){
    GIMS_MultiLineString *fresh = new GIMS_MultiLineString(this->size);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_LineString *));
    return fresh;
}

GIMS_Geometry *GIMS_MultiLineString::clipToBox(GIMS_BoundingBox *box){
    GIMS_MultiLineString clipped = NULL;
    for(int i=0; i<this->size; i++){
        GIMS_MultiLineString *partial = this->list[i]->clipToBox(box);
        if(partial != NULL){
            if(clipped == NULL)
                clipped = new GIMS_MultiLineString(1);
            clipped->append(partial);
        }
    }
    return clipped;
}

void GIMS_MultiLineString::append(GIMS_Point *p){
    this->size += 1;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_LineString **)realloc(this->list, this->size*sizeof(GIMS_LineString *));
        this->allocatedSize = size;
    }
    this->list[this->size-1] = p;
}

GIMS_MultiLineString::GIMS_MultiLineString(int size){
    this->type = MULTILINESTRING;
    this->size = this->allocatedSize = size;
    this->list = (LineString **)malloc(size * sizeof(LineString *));   
}

GIMS_MultiLineString::GIMS_MultiLineString(){
    this->type = MULTILINESTRING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
}

GIMS_MultiLineString::~GIMS_MultiLineString(){
    free(this->list);
}

