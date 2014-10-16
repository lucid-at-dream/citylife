#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "GIMSGeometry.hpp"

using namespace GIMSGeometry;

enum RelStatus {
    TRUE_RELATIONSHIP,
    FALSE_RELATIONSHIP,
    UNDECIDED_RELATIONSHIP
};

class GIMS_DataStruct {
  public:
    /*Functions that take care of the construction and maintenance of the structure*/
    virtual void  build  (GIMSGeometry *);
    virtual void  insert (GIMSGeometry *);
    virtual void  remove (GIMSGeometry *);
    virtual void *search (GIMSGeometry *);
    
    /*Follow the operations between the data structure and a given geometry*/
    virtual RelStatus intersects_g  ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelStatus meets_g       ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelStatus contains_g    ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelStatus isContained_g ( GIMSGeometry *result, GIMSGeometry *);
    
    /*Retrieve all geometry elements that are partially or totally contained
      in a given bounding box*/
    virtual RelStatus isBoundedBy ( GIMSGeometry *result, GIMSBoundingBox *);
    
    /*Allocation & Deallocation*/
    virtual void GIMS_DataStruct (GIMSBoundingBox *domain);
    virtual void ~GIMS_DataStruct (void);
};

#endif
