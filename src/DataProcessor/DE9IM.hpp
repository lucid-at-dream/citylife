#ifndef _DE9IM_HPP_
#define _DE9IM_HPP_

#include <list>

#include "Geometry.hpp"
#include <map>

using namespace std;
using namespace GIMS_GEOMETRY;

#define DIM_INTERSECT(x) (((((x) & INTERSECT) >> 0)-1))
#define DIM_II(x)        (((((x) & II       ) >> 2)-1))
#define DIM_EI(x)        (((((x) & EI       ) >> 4)-1))
#define DIM_IE(x)        (((((x) & IE       ) >> 6)-1))
#define DIM_EB(x)        (((((x) & EB       ) >> 8)-1))
#define DIM_BE(x)        (((((x) & BE       ) >> 10)-1))

enum IntersectionType {
    INTERSECT = 3<<0, /*geometries that intersect the query geometry*/
    II        = 3<<2, /*geometries whose interior intersects query's interior*/
    EI        = 3<<4, /*geometries whose interior intersects query's exterior*/
    IE        = 3<<6, /*geometries whose exterior intersects query's interior*/
    EB        = 3<<8, /*geometries whose boundary intersects query's exterior*/
    BE        = 3<<10, /*geometries whose exterior intersects query's boundary*/
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