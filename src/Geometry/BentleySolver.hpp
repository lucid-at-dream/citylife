#ifndef _BENTLEYSOLVER_HPP_
#define _BENTLEYSOLVER_HPP_

#include "Geometry.hpp"
#include "SystemBase.hpp"
#include <set>
#include <iostream>
#include <list>
#include <vector>
#include <queue>

using namespace std;

typedef struct _Event{
    GIMS_Point *pt;
    char type;
    GIMS_LineSegment ls;
}Event;

bool   compare      (Event a, Event b);
bool   cmp          (GIMS_LineSegment a, GIMS_LineSegment b);
bool   lscmp        (GIMS_LineSegment &a, GIMS_LineSegment &b);

typedef list<GIMS_LineSegment> lsset;
typedef priority_queue<Event, vector<Event>,decltype(&compare)> evset;

class BentleySolver{
  public:
    double                 inputMLS  (evset &, GIMS_MultiLineString *, int);
    list<GIMS_Geometry *>  bruteforce(GIMS_MultiLineString *, GIMS_MultiLineString *);
    list<GIMS_Geometry *>  linesweep (GIMS_MultiLineString *, GIMS_MultiLineString *);
    list<GIMS_Geometry *>  solve     (GIMS_MultiLineString *, GIMS_MultiLineString *);
};

extern long long int bruteforce_count,
                     linesweep_count;

extern double bruteforce_time,
              linesweep_time;

#endif