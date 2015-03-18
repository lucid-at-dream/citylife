#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "SystemBase.hpp"
#include "Geometry.hpp"
#include "DE9IM.hpp"

using namespace GIMS_GEOMETRY;

class GIMS_DataStruct {
  public:

    /*Functions that take care of the construction and maintenance of the structure*/
    virtual void  insert (GIMS_Geometry *) = 0;
    virtual void  remove (GIMS_Geometry *) = 0;
    virtual void *search (GIMS_Geometry *) = 0;

    /*Returns a 9 intersection matrix model conserning the geometry given as query.
      To the construction of the 9IM, only geometries that pass the filter function
      are considered.*/
    virtual DE9IM *topologicalSearch(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *)) = 0;
};
#endif
