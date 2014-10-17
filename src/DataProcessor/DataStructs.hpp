#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "SystemBase.hpp"
#include "GIMSGeometry.hpp"

using namespace GIMSGEOMETRY;

enum RelStatus {
    TRUE_RELATIONSHIP,
    FALSE_RELATIONSHIP,
    UNDECIDED_RELATIONSHIP
};

class GIMSDataStruct {
  public:

    /*Functions that take care of the construction and maintenance of the structure*/
    virtual void  build  (GIMSGeometry *) = 0;
    virtual void  insert (GIMSGeometry *) = 0;
    virtual void  remove (GIMSGeometry *) = 0;
    virtual void *search (GIMSGeometry *) = 0;
    
    /*Follow the operations between the data structure and a given geometry*/
    virtual RelStatus intersects_g  ( GIMSGeometry *result, GIMSGeometry *) = 0;
    virtual RelStatus meets_g       ( GIMSGeometry *result, GIMSGeometry *) = 0;
    virtual RelStatus contains_g    ( GIMSGeometry *result, GIMSGeometry *) = 0;
    virtual RelStatus isContained_g ( GIMSGeometry *result, GIMSGeometry *) = 0;
    
    /*Retrieve all geometry elements that are partially or totally contained
      in a given bounding box*/
    virtual RelStatus isBoundedBy ( GIMSGeometry *result, GIMSBoundingBox *) = 0;
};

#endif
