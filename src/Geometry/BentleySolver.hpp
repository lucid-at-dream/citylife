#ifndef _BENTLEYSOLVER_HPP_
#define _BENTLEYSOLVER_HPP_

#include "Geometry.hpp"
#include "SystemBase.hpp"
#include <set>
#include <iostream>
#include <list>

using namespace std;

typedef struct _Event{
    GIMS_Point *pt;
    char type;
    GIMS_MultiLineSegment *ls;
}Event;

class BentleySolver{
  
  public:  
    static double getYatX      (GIMS_LineSegment *l);
    static int    cmp_linesegs (GIMS_LineSegment *a, GIMS_LineSegment *b);
    static bool   compare      (Event a, Event b);
    static bool   lscmp        (GIMS_LineSegment **ls1, GIMS_LineSegment **ls2);

    typedef set<GIMS_LineSegment **, decltype(&lscmp)> lsset;
    typedef set<Event, decltype(&compare)> evset;

    GIMS_Point            *report              (list<GIMS_Geometry *> &, GIMS_LineSegment *, GIMS_LineSegment *);
    bool                   newIntersectionEvent(evset &eventQueue, GIMS_Point *int_p, GIMS_LineSegment *a, GIMS_LineSegment *b);
    list<GIMS_Geometry *>  solve               (GIMS_MultiLineString *A, GIMS_MultiLineString *B);
};

#endif