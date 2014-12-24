#include "Geometry.hpp"

GIMS_BoundingBox *GIM_SBoundingBox::clone (){
    return new GIMS_BoundingBox( this->lowerLeft->clone(), this->upperRight->clone() );
}

inline double GIMS_BoundingBox::xlength(){
    return fabs( this->upperRight->x - this->lowerLeft->x );
}

inline double GIMS_BoundingBox::ylength(){
    return fabs( this->upperRight->y - this->lowerLeft->y );
}

inline double GIMS_BoundingBox::minx(){
    return this->lowerLeft->x;
}

inline double GIMS_BoundingBox::maxx(){
    return this->upperRight->y;
}

inline double GIMS_BoundingBox::miny(){
    return this->lowerLeft->y;
}

inline double GIMS_BoundingBox::maxy(){
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
    delete this->lowerLeft;
    delete this->upperRight;
}
