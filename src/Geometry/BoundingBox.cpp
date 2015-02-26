#include "Geometry.hpp"

GIMS_BoundingBox *GIMS_BoundingBox::clone (){
    GIMS_BoundingBox *fresh = new GIMS_BoundingBox( this->lowerLeft->clone(), this->upperRight->clone() );
    fresh->id = this->id;
    return fresh;
}

string GIMS_BoundingBox::toWkt(){
    return "";
}

void GIMS_BoundingBox::deleteClipped(){}

GIMS_Point GIMS_BoundingBox::getCenter(){
    return GIMS_Point( (this->upperRight->x + this->lowerLeft->x)/2.0,
                       (this->upperRight->y + this->lowerLeft->y)/2.0 );            
}

double GIMS_BoundingBox::xlength(){
    return fabs( this->upperRight->x - this->lowerLeft->x );
}

double GIMS_BoundingBox::ylength(){
    return fabs( this->upperRight->y - this->lowerLeft->y );
}

double GIMS_BoundingBox::minx(){
    return this->lowerLeft->x;
}

double GIMS_BoundingBox::maxx(){
    return this->upperRight->x;
}

double GIMS_BoundingBox::miny(){
    return this->lowerLeft->y;
}

double GIMS_BoundingBox::maxy(){
    return this->upperRight->y;
}

/*Unsupported*/
GIMS_Geometry *GIMS_BoundingBox::clipToBox ( GIMS_BoundingBox * ){
    fprintf(stderr, "Called clipToBox on a bounding box, which is not supported.");
    return NULL;
}

GIMS_BoundingBox::GIMS_BoundingBox ( GIMS_Point *lowerLeft, GIMS_Point *upperRight ) {
    this->type = BOUNDINGBOX;
    this->lowerLeft = lowerLeft;
    this->upperRight = upperRight;
}

GIMS_BoundingBox::~GIMS_BoundingBox(){
}
