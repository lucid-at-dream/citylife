#include "Geometry.hpp"

int GIMS_LineSegment::getPointCount(){
    return 2;
}

void GIMS_LineSegment::deleteClipped(){}

void GIMS_LineSegment::deepDelete(){
    delete this->p1;
    delete this->p2;
    delete this;
}

string GIMS_LineSegment::toWkt(){
    char buff[100];
    sprintf(buff, "LINESTRING(%lf %lf, %lf %lf)", this->p1->x, this->p1->y, this->p2->x, this->p2->y);
    return buff;
}

GIMS_LineSegment *GIMS_LineSegment::clone () {
    GIMS_LineSegment *fresh = new GIMS_LineSegment( this->p1->clone(), this->p2->clone() );
    fresh->id = this->id;
    return fresh;
}

GIMS_LineSegment::GIMS_LineSegment(){
    this->type = LINESEGMENT;
    this->id = 0;
}

GIMS_LineSegment::GIMS_LineSegment ( GIMS_Point *p1, GIMS_Point *p2 ){
    this->id = 0;
    this->type = LINESEGMENT;
    this->p1 = p1;
    this->p2 = p2;
}

GIMS_LineSegment::~GIMS_LineSegment() {
}

/*returns the point contained in this line segment that is both inside the give
  bounding box "range" and is closer to point "pt" than any other point in the
  line segment. it is assumed that the line segment allways has some portion of 
  it inside the given range.*/
GIMS_Point GIMS_LineSegment::closestPointWithinRange(GIMS_BoundingBox *range, GIMS_Point *pt){

    bool p1inside = this->p1->isInsideBox(range),
         p2inside = this->p2->isInsideBox(range);

    /*if this line segment is fully within the given range we can solve the 
      unconstrained version of the problem.*/
    if ( p1inside && p2inside )
        return pt->getClosestPoint(this);

    /*otherwise we must calculate the clipped line segment and use it as input to
      the unconstrained version of the problem.*/

    GIMS_LineSegment clipped;
    GIMS_Point p1, p2;

    /*these are the square limits*/
    double ymax = range->upperRight->y,
           ymin = range->lowerLeft->y,
           xmax = range->upperRight->x,
           xmin = range->lowerLeft->x;

    /*if line is vertical*/
    if ( this->p1->x == this->p2->x ) {
        if ( p1inside ) {
            double int_y = this->p1->y > this->p2->y ? ymin : ymax;
            p1 = *(this->p1);
            p2 = GIMS_Point(this->p1->x, int_y);
        } else if ( p2inside ) {
            double int_y = this->p2->y > this->p1->y ? ymin : ymax;
            p1 = GIMS_Point(this->p1->x, int_y);
            p2 = *(this->p2);
        } else {
            p1 = GIMS_Point(this->p1->x, ymin);
            p2 = GIMS_Point(this->p1->x, ymax);
        }
        
    /*if line is horizontal*/
    } else if ( this->p1->y == this->p2->y ) {
        if ( p1inside ) {
            double int_x = this->p1->x > this->p2->x ? xmin : xmax;
            p1 = *(this->p1);
            p2 = GIMS_Point(int_x, this->p1->y);
            
        } else if ( p2inside ) {
            double int_x = this->p2->x > this->p1->x ? xmin : xmax;
            p1 = GIMS_Point(int_x, this->p1->y);
            p2 = *(this->p2);
            
        } else {
            p1 = GIMS_Point(xmin, this->p1->y);
            p2 = GIMS_Point(xmax, this->p1->y);
        }

    /*given that we've tested for both vertical and horizontal lines, we may now
      assume that the line segment has a declive different from zero.*/
    }else{

        /*for each line that forms the bounding box, compute the value of t in
          the line equation l(t) = p1 + t*(p2-p1), that results in an intersection.*/
        
        double t_int_ymax = (ymax - this->p1->y) / (this->p2->y - this->p1->y),
               t_int_ymin = (ymin - this->p1->y) / (this->p2->y - this->p1->y),
               t_int_xmax = (xmax - this->p1->x) / (this->p2->x - this->p1->x),
               t_int_xmin = (xmin - this->p1->x) / (this->p2->x - this->p1->x);
        
        /*there are at most two of this values of t within the range [0,1], that
          is, contained in the line segment. There's also at least one within 
          that range because otherwise both line segment endpoints would be 
          contained in the bounding box and we already ruled out that case.*/

        /*we now compute the clipped line segment endpoints. One of them may be
          a line segment endpoint itself. Therefore we initialize accordingly.*/
        if( p1inside )
            p1 = *(this->p1);
        else if( p2inside )
            p1 = *(this->p2);
        bool p1found = p1inside || p2inside;

        /*For each value of "t", we check if the corresponding point is within 
          the given range. Also, to prevent double counting intersection points
          that lie in the square's corner we check if a similar point hasn't 
          been counted*/
        double t_values[] = {t_int_ymax, t_int_ymin, t_int_xmax, t_int_xmin};
        for (double t : t_values) {
            //outside the line segment.
            if( t < 0 || t > 1 )
                continue;

            GIMS_Point ipt = GIMS_Point(this->p1->x + t * (this->p2->x - this->p1->x), 
                                        this->p1->y + t * (this->p2->y - this->p1->y));

            //the resulting point is outside the given range.
            if( ipt.x > xmax || ipt.x < xmin ||
                ipt.y > ymax || ipt.y < ymin )
                continue;

            //right now we now that the resulting point is a valid intersection 
            //point. Never the less we still have to figure out if it's been 
            //previously counted or not.
            if( !p1found ){
                p1 = ipt;
                p1found = true;
            }else if( !ipt.equals(&p1) )
                p2 = ipt;
        }
    }

    clipped = GIMS_LineSegment(&p1, &p2);
    return pt->getClosestPoint(&clipped);
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
int GIMS_LineString::getPointCount(){
    return size;
}

void GIMS_LineString::deepDelete(){
    if(this->list != NULL)
        for(int i=0; i<this->size; i++)
            delete this->list[i];
    delete this;
}

void GIMS_LineString::deleteClipped(){
    delete this;
}

GIMS_LineString::GIMS_LineString (int size){
    this->type = LINESTRING;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
    this->allocatedSize = size;
    this->size = 0;
    this->id = 0;
}

GIMS_LineString::GIMS_LineString (){
    this->type = LINESTRING;
    this->list = NULL;
    this->allocatedSize = 0;
    this->size = 0;
    this->id = 0;
}

GIMS_LineString::~GIMS_LineString (){
    if(this->list != NULL)
        free(this->list);
}

string GIMS_LineString::toWkt(){
    string wkt = string("LINESTRING(");
    char buff[100];
    for(int i=0; i<this->size; i++){
        sprintf(buff, "%lf %lf", this->list[i]->x, this->list[i]->y);
        wkt += string(buff) + ( i < this->size - 1 ? string(",") : string(")") );
    }
    return wkt;
}

GIMS_LineString *GIMS_LineString::clone (){
    GIMS_LineString *newList = new GIMS_LineString(this->allocatedSize);
    newList->size = this->size;
    memcpy(newList->list, this->list, this->size*sizeof(GIMS_Point *));
    newList->id = this->id;
    return newList;
}

GIMS_Geometry *GIMS_LineString::clipToBox (GIMS_BoundingBox *box){
    GIMS_MultiLineString *clipped = NULL;
    GIMS_LineString *partial = NULL;

    for( int i = 0; i<this->size-1; i++ ){
        GIMS_LineSegment segment = this->getLineSegment(i);
        if( segment.clipToBox(box) != NULL ){
            if(partial == NULL){
                partial = new GIMS_LineString(2);
                partial->id = this->id;
                partial->appendPoint(segment.p1);
            }
            partial->appendPoint(segment.p2);
        }else if(partial != NULL){
            if(clipped == NULL){
                clipped = new GIMS_MultiLineString(1);
                clipped->id = this->id;
            }
            clipped->append(partial);
            partial = NULL;
        }
    }

    if(partial != NULL){
        if(clipped == NULL){
            clipped = new GIMS_MultiLineString(1);
            clipped->id = this->id;
        }
        clipped->append(partial);
    }

    return clipped;
}

GIMS_LineSegment GIMS_LineString::getLineSegment (int index){
    if( index+1 >= size )
        printf("reading uninitialized value at line string!!\n");
    
    GIMS_LineSegment ls = GIMS_LineSegment( this->list[index], this->list[index+1]);
    ls.id = this->id;
    return ls;
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
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
    this->id = 0;
}

GIMS_Ring::GIMS_Ring(){
    this->type = RING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
    this->id = 0;
}



/*The MultiLineString class.*/
int GIMS_MultiLineString::getPointCount(){
    int total = 0;
    for(int i=0; i<size; i++)
        total += list[i]->size;
    return total;
}

void GIMS_MultiLineString::deepDelete(){
    if(this->list != NULL)
        for(int i=0; i<this->size; i++)
            this->list[i]->deepDelete();
    delete this;
}

void GIMS_MultiLineString::deleteClipped(){
    if(this->list != NULL)
        for(int i=0; i<this->size; i++){
            this->list[i]->deleteClipped();
        }
    delete this;
}

string GIMS_MultiLineString::toWkt(){
    string wkt = string("MULTILINESTRING(");
    char buff[100];
    
    for(int i=0; i<this->size; i++){
        for(int j=0; j<this->list[i]->size; j++){
            if(j==0)
                wkt += string("(");
            sprintf(buff, "%lf %lf", this->list[i]->list[j]->x, this->list[i]->list[j]->y);
            wkt += string(buff) + ( j < this->list[i]->size - 1 ? string(",") : string(")") );
        }
        wkt += i < this->size - 1 ? "," : ")";
    }

    return wkt;
}

GIMS_MultiLineString *GIMS_MultiLineString::clone(){
    GIMS_MultiLineString *fresh = new GIMS_MultiLineString(this->size);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_LineString *));
    fresh->id = this->id;
    return fresh;
}

GIMS_Geometry *GIMS_MultiLineString::clipToBox(GIMS_BoundingBox *box){
    GIMS_MultiLineString *clipped = NULL;
    for(int i=0; i<this->size; i++){
        GIMS_MultiLineString *partial = (GIMS_MultiLineString *)this->list[i]->clipToBox(box);
        if(partial != NULL){
            if(clipped == NULL){
                clipped = new GIMS_MultiLineString(1);
                clipped->id = this->id;
            }
            clipped->merge(partial);
            delete partial;
        }
    }
    return clipped;
}

void GIMS_MultiLineString::merge(GIMS_MultiLineString *mls){
    int prevSize = this->size;
    this->size = this->size + mls->size;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_LineString **)realloc(this->list, this->size*sizeof(GIMS_LineString *));
        this->allocatedSize = this->size;
    }
    memcpy(this->list+prevSize, mls->list, mls->size*sizeof(GIMS_LineString *));
}

void GIMS_MultiLineString::append(GIMS_LineString *l){
    this->size += 1;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_LineString **)realloc(this->list, this->size*sizeof(GIMS_LineString *));
        this->allocatedSize = size;
    }
    this->list[this->size-1] = l;
}

GIMS_MultiLineString::GIMS_MultiLineString(int size){
    this->type = MULTILINESTRING;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_LineString **)malloc(size * sizeof(GIMS_LineString *));   
    this->id = 0;
}

GIMS_MultiLineString::GIMS_MultiLineString(){
    this->type = MULTILINESTRING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
    this->id = 0;
}

GIMS_MultiLineString::~GIMS_MultiLineString(){
    if(this->list != NULL)
        free(this->list);
}



