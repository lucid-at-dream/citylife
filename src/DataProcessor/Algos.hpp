#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "GIMSGeometry.hpp"

/*Operations between the data structure and a given geometry.
  These functions assume that the first geometry is already in the data structure*/
virtual RelStatus intersects_g  ( GIMSGeometry *, GIMSGeometry *);
virtual RelStatus meets_g       ( GIMSGeometry *, GIMSGeometry *);
virtual RelStatus contains_g    ( GIMSGeometry *, GIMSGeometry *);
virtual RelStatus isContained_g ( GIMSGeometry *, GIMSGeometry *);
virtual RelStatus isBoundedBy   ( GIMSGeometry *, GIMSBoundingBox *);

/*Allocation & Deallocation*/
virtual void GIMS_DataStruct (GIMSBoundingBox *domain);
virtual void ~GIMS_DataStruct (void);

#endif
