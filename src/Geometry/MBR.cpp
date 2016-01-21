#include "Geometry.hpp"

double GIMS_MBR::getArea(){
    return this->box->xlength() * this->box->ylength();
}

double GIMS_MBR::getFalseArea(){
    return falsearea;
}

bool GIMS_MBR::containsApproximation( GIMS_Approximation *appr ){
    GIMS_MBR *other = (GIMS_MBR *)appr;
    return other->box->isInside( this->box );
}


bool GIMS_MBR::isDisjoint(GIMS_BoundingBox *other){
    return this->box->isDisjoint(other);
}

bool GIMS_MBR::isInside(GIMS_BoundingBox *other){
    return this->box->isInside(other);
}

bool GIMS_MBR::containsPoint( GIMS_Point *p){
    return p->isInsideBox( this->box );
}

bool GIMS_MBR::isDisjointFromApproximation( GIMS_Approximation *other ){
    return this->isDisjoint( ((GIMS_MBR *)other)->box );
}

appr_intersection GIMS_MBR::intersection(GIMS_Approximation *other){

    GIMS_BoundingBox *A = this->box, *B = ((GIMS_MBR *)(other))->box;

    double x_overlap = MAX(0, MIN(A->upperRight->x, B->upperRight->x) -
                              MAX(A->lowerLeft->x,  B->lowerLeft->x ) );

    double y_overlap = MAX(0, MIN(A->upperRight->y, B->upperRight->y) -
                              MAX(A->lowerLeft->y,  B->lowerLeft->y ) );

    appr_intersection idata;
    idata.area = x_overlap * y_overlap;
    idata.intersects = !this->isDisjoint(((GIMS_MBR *)(other))->box);
    return idata;
}

GIMS_MBR::GIMS_MBR(GIMS_Polygon *p){
    this->box = p->getExtent();
    this->falsearea = this->getArea() - p->area();
    this->N = 0;
    this->hull = NULL;
}

GIMS_MBR::~GIMS_MBR(){}
