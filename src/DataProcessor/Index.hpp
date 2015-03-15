#ifndef DATASTRUCTS_HPP
#define DATASTRUCTS_HPP

#include "Geometry.hpp"
#include "DataStructs.hpp"

class GIMS_Index{

  private:
    GIMS_DataStruct *index;

  public:
    /*
     * INDEX MANIPULATION FUNCTIONS
     */

    /*inserts the given geometry in the index*/
    void insert(GIMS_Geometry *);
    
    /*removes the given geometry from the index*/
    void remove(GIMS_Geometry *);


    /*
     * TOPOLOGICAL RELATIONSHIP SEARCH FUNCTIONS
     */

    /*returns all geometries in the data structure for which function "filter" 
      returns true and that intersect the given geometry*/
    AVLTree<long, GIMS_Geometry *> *intersect  (GIMSGeometry *geom, int(*filter)(GIMS_Geometry *));

    /*returns all geometries in the data structure for which function "filter" 
      returns true and that meet the given geometry*/
    AVLTree<long, GIMS_Geometry *> *meet       (GIMSGeometry *geom, int(*filter)(GIMS_Geometry *));

    /*returns all geometries in the data structure for which function "filter" 
      returns true and that overlap with the given geometry*/
    AVLTree<long, GIMS_Geometry *> *overlap    (GIMSGeometry *geom, int(*filter)(GIMS_Geometry *));

    /*returns all geometries in the data structure for which function "filter" 
      returns true and that cover the given geometry*/
    AVLTree<long, GIMS_Geometry *> *covers     (GIMSGeometry *geom, int(*filter)(GIMS_Geometry *));

    /*returns all geometries in the data structure for which function "filter" 
      returns true and that are covered by the given geometry*/
    AVLTree<long, GIMS_Geometry *> *isCoveredBy(GIMSGeometry *geom, int(*filter)(GIMS_Geometry *));

};

#endif
