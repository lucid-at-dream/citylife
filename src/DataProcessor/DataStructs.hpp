#include "GIMSGeometry.hpp"

enum RelationshipStatus {
    TRUE_RELATIONSHIP,
    FALSE_RELATIONSHIP,
    UNDECIDED_RELATIONSHIP
};

class GIMS_DataStruct {
  public:
    /*Functions that take care of the construction and maintenance of the structure*/
    virtual void build (GIMSGeometry *);
    virtual void insert (GIMSGeometry *);
    
    /*Follow the operations between the data structure and a given geometry*/
    virtual RelationshipStatus intersects_g ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelationshipStatus meets_g      ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelationshipStatus contains_g   ( GIMSGeometry *result, GIMSGeometry *);
    virtual RelationshipStatus isContained_g ( GIMSGeometry *result,
            GIMSGeometry *);
            
    /*Retrieve all geometry elements that are partially or totally contained
      in a given bounding box*/
    virtual RelationshipStatus isBoundedBy  ( GIMSGeometry *result,
            GIMSBoundingBox *);
            
    /*Allocation & Deallocation*/
    virtual void GIMS_DataStruct (GIMSBoundingBox *domain);
    virtual void ~GIMS_DataStruct (void);
};
