#include "BoundingBox.hpp"

GIMSBoundingBox *GIMSBoundingBox::clone (){
    return new GIMSBoundingBox( this->lowerLeft->clone(), this->upperRight->clone() );
}

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
    return NULL;
}

GIMSBoundingBox::GIMSBoundingBox ( GIMSPoint *lowerLeft, GIMSPoint *upperRight ) {
    this->lowerLeft = lowerLeft;
    this->upperRight = upperRight;
    this->type = BOUNDINGBOX;
    this->renderCount = 0;
}

GIMSBoundingBox::~GIMSBoundingBox(){
    delete this->lowerLeft;
    delete this->upperRight;
}
