#ifndef _DE9IM_HPP_
#define _DE9IM_HPP_

#include <list>

#include "Geometry.hpp"
#include <map>

using namespace std;
using namespace GIMS_GEOMETRY;

#define DIM_INTERSECT(x) (((((x)&INTERSECT) >> 0) - 1))
#define DIM_II(x) (((((x)&II) >> 2) - 1))
#define DIM_EI(x) (((((x)&EI) >> 4) - 1))
#define DIM_IE(x) (((((x)&IE) >> 6) - 1))
#define DIM_EB(x) (((((x)&EB) >> 8) - 1))
#define DIM_BE(x) (((((x)&BE) >> 10) - 1))

enum IntersectionType {
        INTERSECT = 3 << 0, /*geometries that intersect the query geometry*/
        II = 3 << 2, /*geometries whose interior intersects query's interior*/
        EI = 3 << 4, /*geometries whose interior intersects query's exterior*/
        IE = 3 << 6, /*geometries whose exterior intersects query's interior*/
        EB = 3 << 8, /*geometries whose boundary intersects query's exterior*/
        BE = 3 << 10, /*geometries whose exterior intersects query's boundary*/
};

typedef map<long long, unsigned int> matrix_t;

class DE9IM {
    public:
        GIMS_Geometry *query;
        matrix_t matrix;

        DE9IM(GIMS_Geometry *query);
        ~DE9IM();

        matrix_t::iterator getMatrixIndex(long long id);

        matrix_t::iterator setIntersect(long long id, unsigned int dim);
        matrix_t::iterator setII(long long id, unsigned int dim);
        matrix_t::iterator setEI(long long id, unsigned int dim);
        matrix_t::iterator setIE(long long id, unsigned int dim);
        matrix_t::iterator setEB(long long id, unsigned int dim);
        matrix_t::iterator setBE(long long id, unsigned int dim);

        void setIntersect(matrix_t::iterator &, unsigned int dim);
        void setII(matrix_t::iterator &, unsigned int dim);
        void setEI(matrix_t::iterator &, unsigned int dim);
        void setIE(matrix_t::iterator &, unsigned int dim);
        void setEB(matrix_t::iterator &, unsigned int dim);
        void setBE(matrix_t::iterator &, unsigned int dim);

        list<long> equals();
        list<long> disjoint();
        list<long> meets();
        list<long> contains();
        list<long> covers();
        list<long> intersects();
        list<long> within();
        list<long> coveredBy();
        list<long> overlaps();
};

#endif