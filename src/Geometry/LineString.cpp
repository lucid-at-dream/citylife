#include "LineString.hpp"

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

GIMSGeometry *GIMS_LineSegment::clipToBox ( GIMSBoundingBox *box ){

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

/*returns a new edge where the endpoints are the actual intersection points with
  the bounding square */
GIMS_LineSegment *GIMS_LineSegment::trimToBBox (GIMSBoundingBox *box) {

    bool p1inside = this->p1->isInsideBox(box),
         p2inside = this->p2->isInsideBox(box);
         
    /*if both endpoints lie inside the square*/
    if ( p1inside && p2inside ) {
        return this->clone();
    }
    
    /*these are the square limits*/
    double ymax = box->upperRight->y,
           ymin = box->lowerLeft->y,
           xmax = box->upperRight->x,
           xmin = box->lowerLeft->x;
           
    /*if line is vertical*/
    if ( this->p1->x == this->p2->x ) {
        if ( p1inside ) {
            double int_y = this->p1->y > this->p2->y ? ymin : ymax;
            return new GIMS_LineSegment ( this->p1->clone(), new GIMS_Point( this->p1->x, int_y ) );
            
        } else if ( p2inside ) {
            double int_y = this->p2->y > this->p1->y ? ymin : ymax;
            return new GIMS_LineSegment ( new GIMS_Point ( this->p1->x, int_y ), this->p2->clone() );
            
        } else {
            return new GIMS_LineSegment ( new GIMS_Point ( this->p1->x, ymin ),
                                          new GIMS_Point ( this->p1->x, ymax ) );
        }
        
    /*if line is horizontal*/
    } else if ( this->p1->y == this->p2->y ) {
        if ( p1inside ) {
            double int_x = this->p1->x > this->p2->x ? xmin : xmax;
            return new GIMS_LineSegment ( this->p1->clone(), new GIMS_Point ( int_x, this->p1->y ) );
            
        } else if ( p2inside ) {
            double int_x = this->p2->x > this->p1->x ? xmin : xmax;
            return new GIMS_LineSegment ( new GIMS_Point ( int_x, this->p1->y ), this->p2->clone() );
            
        } else {
            return new GIMS_LineSegment ( new GIMS_Point ( xmin, this->p1->y ),
                                          new GIMS_Point ( xmax, this->p1->y ) );
        }
    }
    
    /*calculate the intersection points and alloc corresponding points*/
    double m = (this->p1->y - this->p2->y) / (this->p1->x - this->p2->x);
    double b = this->p1->y - m * this->p1->x;
    
    double int_ymax = (ymax - b) / m,
           int_ymin = (ymin - b) / m,
           int_xmax = m * xmax + b,
           int_xmin = m * xmin + b;
           
    GIMS_Point *top_pt    = new GIMS_Point ( int_ymax, ymax ),
              *bottom_pt = new GIMS_Point ( int_ymin, ymin ),
              *left_pt   = new GIMS_Point ( xmin, int_xmin ),
              *right_pt  = new GIMS_Point ( xmax, int_xmax );
             
    /*For each intersection point we check if it is within the square. also, to
      prevent double counting intersection points that lie in the square's
      corner we check if a similar point hasn't been counted*/
    GIMS_Point *int_p1 = p1inside ? this->p1->clone() : NULL;
    GIMS_Point *int_p2 = p2inside ? this->p2->clone() : NULL;
    
    
    GIMS_Point *int_pts[] = {top_pt, bottom_pt, left_pt, right_pt, int_p1, int_p2};
    GIMS_Point *p1 = NULL, *p2 = NULL;

    for (int i = 0; i < 6; i++) {
        if( int_pts[i] == NULL )
            continue;
        if ( !(int_pts[i]->isInsideBox(box) && int_pts[i]->isInsideEdgeOfSameLine(this)) ) {
            delete ( int_pts[i] );
            int_pts[i] = NULL;
        }else{
            if( p1 == NULL )
                p1 = int_pts[i];
            else if( int_pts[i]->x != p1->x && int_pts[i]->y != p1->y )
                p2 = int_pts[i];
            else
                delete int_pts[i];
        }
    }
    
    /*If the edge hits the square exactly on the corner, the intersection is
      a single point and therefore p2 will be set to NULL*/
    if ( p1 != NULL && p2 == NULL ) {
        return new GIMS_LineSegment (p1, p1->clone());
    } else if ( p1 != NULL && p2 != NULL ) {
        return new GIMS_LineSegment (p1, p2);
        
    } else {
        perror ("trimEdge called on a edge that did not intersect the square");
        return NULL;
    }
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

GIMSGeometry *GIMS_LineString::clipToBox (GIMSBoundingBox *box){
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

