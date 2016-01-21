#ifndef _TUNASOLVER_HPP_
#define _TUNASOLVER_HPP_

#include "DBConnection.hpp"
#include "SystemBase.hpp"
#include "Geometry.hpp"
#include "DE9IM.hpp"
#include "conf.hpp"
#include "DebRender.hpp"

class spatialIndex;
using namespace GIMS_GEOMETRY;

/* performs a query on the underlying postGIS database. Note that the query received
 * as argument is not supposed to specify the fields of the SELECT clause. i.e., inside
 * the function the fields are already defined as "SELECT osm_id, way %s", here %s
 * represents the string receiverd as argument. You can specify any filters, sortings, etc.*/
list<GIMS_Geometry *> loadPostgisData( const char *query );
void connectToDatabase();

/* Creates a Polygonal Map Quadtree Index given the domain of interest. */
spatialIndex *createSpatialIndex( GIMS_BoundingBox *extent );

/* reads the configuration file. returns 0 on success and any other value on error. */
int loadConfiguration(int argc, char **argv);

/* render the index */
void renderIndex(DebugRenderable *renderable, GIMS_BoundingBox *extent);

/* some filtering functions */
int filter_bypass(GIMS_Geometry *obj);
int filter_point(GIMS_Geometry *obj);
int filter_polygon(GIMS_Geometry *obj);
int filter_linestring(GIMS_Geometry *obj);

/* Clear Up */
void shutdownTunaSolver();

class spatialIndex : public DebugRenderable{
  public:
    virtual void  insert (GIMS_Geometry *) = 0;
    virtual void  insert (list<GIMS_Geometry *> *geom) = 0;
    virtual void  insert (list<GIMS_Geometry *> &geom) = 0;
    virtual void  remove (GIMS_Geometry *) = 0;

    /* The topological search function returns the intersection matrix for all pairs with the given 
     * query object that pass the filter function. Can be used for more flexible topological relationship
     * verification. */ 
    virtual DE9IM *topologicalSearch(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *)) = 0;

    /* The following functions can be used for topological searching. Considering that the function name
     * represents the topological predicate, given a query object and a function that returns 1 for every
     * indexed objects that is to be analyzed and 0 otherwise, the return value is a list with the ids of
     * the objects that verify the wanted relationships. i.e. <query> <predicate> <object>. */
    list<long> contains(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));
    list<long> covers(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));
    list<long> intersects(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));
    list<long> coveredBy(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));
    list<long> touches(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));
    list<long> overlaps(GIMS_Geometry *query, int(*filter)(GIMS_Geometry *));

    virtual ~spatialIndex() = 0;
};

#endif