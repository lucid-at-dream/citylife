#include "Geometry.hpp"

void GIMS_LineSegment::deleteClipped(){}

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

void GIMS_MultiLineString::deleteClipped(){
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

