#ifndef _BENTLEYSOLVER_HPP_
#define _BENTLEYSOLVER_HPP_

#include "Geometry.hpp"
#include "SystemBase.hpp"
#include "DE9IM.hpp"
#include "Balaban.hpp"
#include <set>
#include <iostream>
#include <list>
#include <vector>
#include <queue>

using namespace std;

typedef struct _Event {
    GIMS_Point *pt;
    char type;
    GIMS_LineSegment ls;
} Event;

typedef struct _BO_Event {
    GIMS_Point *pt;
    char type;
    GIMS_MultiLineSegment *ls;
} BO_Event;

bool compare(Event a, Event b);
bool cmp(GIMS_LineSegment a, GIMS_LineSegment b);
bool lscmp(GIMS_LineSegment &a, GIMS_LineSegment &b);

typedef list<GIMS_LineSegment> lsset;
typedef priority_queue<Event, vector<Event>, decltype(&compare)> evset;

class BentleySolver {
public:
    /*line sweep and bruteforce implementation*/
    double inputMLS(evset &, GIMS_MultiLineString *, int);
    list<GIMS_Geometry *> bruteforce(GIMS_MultiLineString *, GIMS_MultiLineString *);
    list<GIMS_Geometry *> linesweep(GIMS_MultiLineString *, GIMS_MultiLineString *);
    list<GIMS_Geometry *> solve(GIMS_MultiLineString *, GIMS_MultiLineString *);

    /*bentley ottman implementation*/
    static mpf_class getYatX(GIMS_LineSegment *l);
    static int BO_cmp_linesegs(GIMS_LineSegment *a, GIMS_LineSegment *b);
    static bool BO_compare(BO_Event a, BO_Event b);
    static bool BO_lscmp(GIMS_LineSegment **ls1, GIMS_LineSegment **ls2);

    typedef multiset<GIMS_LineSegment **, decltype(&BO_lscmp)> BO_lsset;
    typedef set<BO_Event, decltype(&BO_compare)> BO_evset;

    GIMS_Point *report(list<GIMS_Geometry *> &, GIMS_LineSegment *, GIMS_LineSegment *);
    bool newIntersectionBOEvent(BO_evset &eventQueue, GIMS_Point *int_p, GIMS_LineSegment *a, GIMS_LineSegment *b);
    list<GIMS_Geometry *> bentley(GIMS_MultiLineString *A, GIMS_MultiLineString *B);
};

extern long long int bruteforce_count, linesweep_count;

extern double bruteforce_time, linesweep_time;

#define GET_LSINT_TIME 1

#ifdef GET_LSINT_TIME
extern int lsint_algo;
#endif

#endif
