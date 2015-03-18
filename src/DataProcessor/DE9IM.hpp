#ifndef _DE9IM_HPP_
#define _DE9IM_HPP_

#include <list>

#include "Geometry.hpp"
#include <map>

using namespace std;
using namespace GIMS_GEOMETRY;

enum IntersectionType {
    INTERSECT = 1<<0, /*geometries that intersect the query geometry*/
    II        = 1<<1, /*geometries whose interior intersects query's interior*/
    EI        = 1<<2, /*geometries whose interior intersects query's exterior*/
    IE        = 1<<3, /*geometries whose exterior intersects query's interior*/
    EB        = 1<<4, /*geometries whose boundary intersects query's exterior*/
    BE        = 1<<5, /*geometries whose exterior intersects query's boundary*/
};


class DE9IM{

  public:
    GIMS_Geometry *query;
    map<long long int, unsigned int> matrix;

    DE9IM(GIMS_Geometry *query);
    ~DE9IM();
    
    void setIntersect(long long int id);
    void setII(long long int id);
    void setEI(long long int id);
    void setIE(long long int id);
    void setEB(long long int id);
    void setBE(long long int id);

    list<long> equals();
    list<long> disjoint();
    list<long> meets();
    list<long> contains();
    list<long> covers();
    
    list<long> intersects();
    list<long> within();
    list<long> coveredBy();

    list<long> crosses();
    list<long> overlaps();
};

#endif