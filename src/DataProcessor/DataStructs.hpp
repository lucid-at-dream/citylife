#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "SystemBase.hpp"
#include "Geometry.hpp"

using namespace GIMS_GEOMETRY;

enum RelStatus {
    TRUE_RELATIONSHIP,
    FALSE_RELATIONSHIP,
    UNDECIDED_RELATIONSHIP
};

class GIMS_DataStruct {
  public:

    /*Functions that take care of the construction and maintenance of the structure*/
    virtual void  build  (GIMS_Geometry *) = 0;
    virtual void  insert (GIMS_Geometry *) = 0;
    virtual void  remove (GIMS_Geometry *) = 0;
    virtual void *search (GIMS_Geometry *) = 0;
    
    /*Follow the operations between the data structure and a given geometry*/
    virtual RelStatus intersects_g  ( GIMS_Geometry *result, GIMS_Geometry *) = 0;
    virtual RelStatus meets_g       ( GIMS_Geometry *result, GIMS_Geometry *) = 0;
    virtual RelStatus contains_g    ( GIMS_Geometry *result, GIMS_Geometry *) = 0;
    virtual RelStatus isContained_g ( GIMS_Geometry *result, GIMS_Geometry *) = 0;
    
    /*Retrieve all geometry elements that are partially or totally contained
      in a given bounding box*/
    virtual RelStatus isBoundedBy ( GIMS_Geometry *result, GIMS_BoundingBox *) = 0;
};

#endif
