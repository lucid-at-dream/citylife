#include "BentleySolver.hpp"

#define _BS_PRECISION 1000000
#define THRESHOLD 20
#define THRESHOLD2 1000

list<GIMS_Geometry *> BentleySolver::solve(GIMS_MultiLineString *A, GIMS_MultiLineString *B)
{
    bool disjoint = true;
    for (int i = 0; disjoint && i < A->size; i++) {
        for (int j = 0; disjoint && j < B->size; j++) {
            if (!(A->list[i]->bbox.isDisjoint(&(B->list[j]->bbox))))
                disjoint = false;
        }
    }

    if (disjoint)
        return list<GIMS_Geometry *>();

    int NA = A->getPointCount(), NB = B->getPointCount();

    if (NA < THRESHOLD || NB < THRESHOLD)
        return this->bruteforce(A, B);

    else
        return linesweep(A, B);
}

bool cmp(GIMS_LineSegment a, GIMS_LineSegment b)
{
    return a.osm_id < b.osm_id;
}

bool compare(Event a, Event b)
{
    if (a.pt->x < b.pt->x - ERR_MARGIN) {
        return false;
    } else if (a.pt->x < b.pt->x + ERR_MARGIN && a.pt->x > b.pt->x - ERR_MARGIN) {
        if (a.pt->y < b.pt->y - ERR_MARGIN)
            return false;
        else if (a.pt->y < b.pt->y + ERR_MARGIN && a.pt->y > b.pt->y - ERR_MARGIN) {
            //if x and y are equal, differentiate by event type
            if (a.type < b.type)
                return false;
            else if (b.type < a.type)
                return true;

            //if both x and y are equal and the event type is equal
            //differentiate by the given ID
            else if (a.ls.id < b.ls.id)
                return false;

            //finally, we must distinguish line segments that start at the same point and
            //are in the same set.
            else
                return lscmp(a.ls, b.ls);

        } else
            return true;
    } else
        return true;
}

bool lscmp(GIMS_LineSegment &a, GIMS_LineSegment &b)
{
    if (a.p1->x < b.p1->x)
        return true;
    else if (a.p1->x == b.p1->x) {
        if (a.p1->y < b.p1->y)
            return true;
        else if (a.p1->y > b.p1->y)
            return false;
    } else
        return false;

    //at this point we know that a.p1 equals b.p1
    if (a.p2->x < b.p2->x)
        return true;
    else if (a.p2->x == b.p2->x) {
        if (a.p2->y < b.p2->y)
            return true;
        else if (a.p2->y > b.p2->y)
            return false;
    } else
        return false;

    //if this point is reached, the line segments are equal
    return false;
}

list<GIMS_Geometry *> BentleySolver::bruteforce(GIMS_MultiLineString *A, GIMS_MultiLineString *B)
{
    list<GIMS_Geometry *> intersections;
    for (int i = 0; i < A->size; i++) {
        for (int j = 0; j < A->list[i]->size - 1; j++) {
            for (int k = 0; k < B->size; k++) {
                for (int m = 0; m < B->list[k]->size - 1; m++) {
                    GIMS_LineSegment _a = A->list[i]->getLineSegment(j);
                    GIMS_LineSegment _b = B->list[k]->getLineSegment(m);

                    GIMS_Geometry *aux = _a.intersects(&_b);

                    if (aux != NULL)
                        intersections.push_back(aux);
                }
            }
        }
    }
    return intersections;
}

double BentleySolver::inputMLS(evset &eventQueue, GIMS_MultiLineString *mls, int id)
{
    double max_x = -1e100;
    int count = 1;
    for (int i = 0; i < mls->size; i++) {
        for (int j = 0; j < mls->list[i]->size - 1; j++) {
            Event a, b;
            GIMS_LineSegment ls = mls->list[i]->getLineSegment(j);
            ls.id = id;
            ls.osm_id = count++;

            a.ls = ls;
            b.ls = ls;
            a.type = 0;
            b.type = 1;

            if (ls.p1->x < ls.p2->x) {
                a.pt = ls.p1;
                b.pt = ls.p2;
            } else if (ls.p1->x > ls.p2->x) {
                a.pt = ls.p2;
                b.pt = ls.p1;
            } else {
                if (ls.p1->y < ls.p2->y) {
                    a.pt = ls.p1;
                    b.pt = ls.p2;
                } else if (ls.p1->y > ls.p2->y) {
                    a.pt = ls.p2;
                    b.pt = ls.p1;
                } else {
                    cerr << "degenerate line segment (it's a point)" << endl;
                    continue;
                }
            }

            if (ls.p1->x > max_x)
                max_x = ls.p1->x;
            if (ls.p2->x > max_x)
                max_x = ls.p2->x;

            eventQueue.push(a);
            eventQueue.push(b);
        }
    }
    return max_x;
}

GIMS_Point *getrp(GIMS_LineSegment &ls)
{
    if (ls.p1->x < ls.p2->x)
        return ls.p2;
    else if (ls.p1->x > ls.p2->x)
        return ls.p1;
    else {
        if (ls.p1->y < ls.p2->y)
            return ls.p2;
        else if (ls.p1->y > ls.p2->y)
            return ls.p1;
    }
    return 0;
}

bool fktp_cmp(GIMS_LineSegment &a, GIMS_LineSegment &b)
{
    GIMS_Point *arp = getrp(a), *brp = getrp(b);

    if (arp->x < brp->x)
        return true;
    else if (arp->x > brp->x)
        return false;
    else {
        if (arp->y < brp->y)
            return true;
        else if (arp->y > brp->y)
            return false;
    }
    return false;
}

void insert(lsset &l, GIMS_LineSegment &ls)
{
    for (lsset::iterator it = l.begin(); it != l.end(); it++) {
        if (!fktp_cmp(*it, ls)) {
            l.insert(it, ls);
            return;
        }
    }
    l.push_back(ls);
}

list<GIMS_Geometry *> BentleySolver::linesweep(GIMS_MultiLineString *A, GIMS_MultiLineString *B)
{
    list<GIMS_Geometry *> intersections;
    evset eventQueue(&compare);

    double last_x_A = this->inputMLS(eventQueue, A, 1);
    double last_x_B = this->inputMLS(eventQueue, B, 2);
    double last_x = MIN(last_x_A, last_x_B); //after this value of X, no intersections happen.

    lsset red, blue;

    Event event;
    while (!eventQueue.empty()) {
        event = eventQueue.top();
        eventQueue.pop();
        if (event.pt->x > last_x)
            break;

        if (event.type == 0) {
            if (event.ls.id == 1) { //red
                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                    GIMS_Geometry *g = event.ls.intersects((&(*it)));
                    if (g != NULL)
                        intersections.push_back(g);
                }
                insert(red, event.ls);
            } else { //blue
                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                    GIMS_Geometry *g = event.ls.intersects((&(*it)));
                    if (g != NULL)
                        intersections.push_back(g);
                }
                insert(blue, event.ls);
            }
        }

        else if (event.type == 1) {
            if (event.ls.id == 1) { //red
                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                    if (lscmp(*it, event.ls) == 0) {
                        red.erase(it);
                        break;
                    }
                }
            } else { //blue
                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                    if (lscmp(*it, event.ls) == 0) {
                        blue.erase(it);
                        break;
                    }
                }
            }
            //event.ls.deepDelete();
        }
    }

    return intersections;
}

/*Le Bentley-Ottman implementation*/

/*Le Bentley-Ottman implementation*/

double sweepLineX = 0;
double sweepLineY = 0;

mpf_class BentleySolver::getYatX(GIMS_LineSegment *l)
{
    mpf_class p1x = l->p1->x, p1y = l->p1->y, p2x = l->p2->x, p2y = l->p2->y;

    if (p1x == p2x) {
        if (p1x >= sweepLineX) {
            mpf_class miny = MIN(p1y, p2y), maxy = MAX(p1y, p2y);
            mpf_class fy = MIN(MAX(sweepLineY - 1e-3, miny), maxy);
            return fy.get_d();
        } else {
            return (MAX(p1y, p2y)).get_d();
        }
    }

    if (p1y == p2y)
        return p1y.get_d();

    mpf_class m = (p1y - p2y) / (p1x - p2x), b = p1y - m * p1x;

    mpf_class fy = m * sweepLineX + b;

    return fy.get_d();
}

int BentleySolver::BO_cmp_linesegs(GIMS_LineSegment *a, GIMS_LineSegment *b)
{
    if (a->p1->x < b->p1->x)
        return 1;
    else if (a->p1->x == b->p1->x) {
        if (a->p1->y < b->p1->y)
            return 1;
        else if (a->p1->y > b->p1->y)
            return -1;
    } else
        return -1;

    //at this point we know that a->p1 equals b->p1
    if (a->p2->x < b->p2->x)
        return 1;
    else if (a->p2->x == b->p2->x) {
        if (a->p2->y < b->p2->y)
            return 1;
        else if (a->p2->y > b->p2->y)
            return -1;
    } else
        return -1;

    //if this point is reached, the line segments are equal
    return 0;
}

bool BentleySolver::BO_compare(BO_Event a, BO_Event b)
{
    if (a.pt->x < b.pt->x - ERR_MARGIN) {
        return true;

    } else if (a.pt->x < b.pt->x + ERR_MARGIN && a.pt->x > b.pt->x - ERR_MARGIN) {
        if (a.pt->y < b.pt->y - ERR_MARGIN)
            return true;
        else if (a.pt->y < b.pt->y + ERR_MARGIN && a.pt->y > b.pt->y - ERR_MARGIN) {
            if (a.type == 0 && b.type > 0)
                return true;
            else if (b.type == 0 && a.type > 0)
                return false;
            else if (a.type == 2 && b.type == 1)
                return true;
            else if (b.type == 2 && a.type == 1)
                return false;
        } else
            return false;
    } else
        return false;

    //at this point, we know that the events start at the same point
    //and take the same precedence. Therefore, the order is not important.
    //Never the less, we must assure a predictable ordering if the events are
    //in fact different.

    //We need to be careful defining "different" events at the same point. They
    //are different only if none of the linesegments of "a" is present in "b".
    for (int i = 0; i < a.ls->size; i++) {
        for (int j = 0; j < b.ls->size; j++) {
            if (*(a.ls->list[i]) == *(b.ls->list[j]))
                return false;
        }
    }

    return BO_cmp_linesegs(a.ls->list[0], b.ls->list[0]) > 0 ? true : false;
}

bool BentleySolver::BO_lscmp(GIMS_LineSegment **ls1, GIMS_LineSegment **ls2)
{
    GIMS_LineSegment *a = *ls1, *b = *ls2;

    mpf_class ay = getYatX(a), by = getYatX(b);

    if (ay < by) {
        return true;
    } else if (ay == by) {
        return BO_cmp_linesegs(a, b) > 0 ? true : false;
    }

    return false;
}

GIMS_Point *BentleySolver::report(list<GIMS_Geometry *> &intersections, GIMS_LineSegment *a, GIMS_LineSegment *b)
{
    GIMS_Geometry *g = a->intersects(b);

    if (g == NULL) {
        return NULL;
    }

    if (g->type == LINESEGMENT) {
        if (a->id != b->id)
            intersections.push_back(g);
        return NULL;
    }

    if (((GIMS_Point *)g)->x >= sweepLineX - 1e-3)
        return (GIMS_Point *)g;
    return NULL;
}

bool BentleySolver::newIntersectionBOEvent(BO_evset &eventQueue, GIMS_Point *int_p, GIMS_LineSegment *a,
                                           GIMS_LineSegment *b)
{
    GIMS_MultiLineSegment *mls = new GIMS_MultiLineSegment(2);
    mls->append(a);
    mls->append(b);

    BO_Event e = { int_p, 2, mls };

    bool add_a = true, add_b = true;
    BO_evset::iterator it = eventQueue.find(e);
    if (it != eventQueue.end()) {
        for (int i = 0; i < it->ls->size; i++) {
            if (*a == *(it->ls->list[i]))
                add_a = false;
            if (*b == *(it->ls->list[i]))
                add_b = false;
        }

        if (add_a)
            it->ls->append(a);
        if (add_b)
            it->ls->append(b);

        return true;

    } else {
        eventQueue.insert(e);
        return false;
    }
}

list<GIMS_Geometry *> BentleySolver::bentley(GIMS_MultiLineString *A, GIMS_MultiLineString *B)
{
    list<GIMS_Geometry *> intersections;

    BO_evset eventQueue(&BO_compare);

    int nextId = 0;

    GIMS_MultiLineSegment *mls;
    GIMS_MultiLineString *src[2] = { A, B };

    for (GIMS_MultiLineString *aux : src) {
        nextId++;
        for (int i = 0; i < aux->size; i++) {
            for (int j = 0; j < aux->list[i]->size - 1; j++) {
                BO_Event a, b;
                GIMS_LineSegment ls = aux->list[i]->getLineSegment(j);

                a.pt = ls.p1;
                b.pt = ls.p2;
                if (BO_compare(a, b)) {
                    a.type = 0;
                    b.type = 1;
                } else {
                    a.type = 1;
                    b.type = 0;
                }

                GIMS_LineSegment *ls_copy = ls.clone();
                ls_copy->id = nextId;

                mls = new GIMS_MultiLineSegment(1);
                mls->append(ls_copy);
                a.ls = mls;

                mls = new GIMS_MultiLineSegment(1);
                mls->append(ls_copy);
                b.ls = mls;

                eventQueue.insert(a);
                eventQueue.insert(b);
            }
        }
    }

    //create a binary search tree for line segments
    BO_lsset T(&BO_lscmp);

    BO_Event event;
    GIMS_Point *int_p;

    while (!eventQueue.empty()) {
        //fetch the next event from the queue
        event = *(eventQueue.begin());
        eventQueue.erase(eventQueue.begin());

        //update the sweeping lines, we need the vertical for vertical line segments.
        sweepLineX = event.pt->x;
        sweepLineY = event.pt->y;

        //the event refers to a left endpoint
        if (event.type == 0) {
            GIMS_LineSegment *ls = event.ls->list[0];

            sweepLineX -= 1e-3;
            BO_lsset::iterator r = T.insert(event.ls->list);
            sweepLineX += 1e-3;

            BO_lsset::iterator above = r;
            above++;
            BO_lsset::iterator below = r;
            if (below == T.begin())
                below = T.end();
            else
                below--;

            if (above != T.end()) {
                int_p = report(intersections, ls, **above);
                if (int_p != NULL)
                    newIntersectionBOEvent(eventQueue, int_p, ls, **above);
            }

            if (below != T.end()) {
                int_p = report(intersections, ls, **below);
                if (int_p != NULL)
                    newIntersectionBOEvent(eventQueue, int_p, **below, ls);
            }

            //the event refers to a right endpoint
        } else if (event.type == 1) {
            GIMS_LineSegment *ls = event.ls->list[0];

            sweepLineX += 1e-3;
            BO_lsset::iterator it = T.find(&ls);

            if (it == T.end())
                continue;

            BO_lsset::iterator above = it;
            above++;
            BO_lsset::iterator below = it;
            if (below == T.begin())
                below = T.end();
            else
                below--;
            T.erase(it);
            sweepLineX -= 1e-3;

            if (above != T.end() && below != T.end()) {
                int_p = report(intersections, **above, **below);
                if (int_p != NULL) {
                    newIntersectionBOEvent(eventQueue, int_p, **below, **above);
                }
            }

            //free up the memory allocated for the line segment
            event.ls->deepDelete();

            //the event refers to an intersection point
        } else if (event.type == 2) {
            int previd = event.ls->list[0]->id;
            for (int i = 1; i < event.ls->size; i++) {
                if (event.ls->list[i]->id != previd) {
                    intersections.push_back(event.pt->clone());
                    break;
                }
            }

            sweepLineX -= 1e-3;
            //swap positions of s and t in T
            BO_lsset::iterator s = T.end(), t = T.begin(), aux, aux_s, aux_t;
            s--;

            //find the first and last line segments
            bool bogus_event = false;
            for (int i = 0; i < event.ls->size; i++) {
                aux = T.find(&(event.ls->list[i]));

                if (aux == T.end()) {
                    for (BO_lsset::iterator T_it = T.begin(); T_it != T.end(); T_it++) {
                        GIMS_LineSegment *A = event.ls->list[i];
                        GIMS_LineSegment *B = **T_it;

                        if (A->p1->equals(B->p1) && A->p2->equals(B->p2)) {
                            aux = T_it;
                            break;
                        }
                    }
                    bogus_event = true;
                    break;
                }

                if (BO_lscmp(*aux, *s))
                    s = aux;
                if (BO_lscmp(*t, *aux))
                    t = aux;
            }

            if (bogus_event)
                continue;

            sweepLineX += 1e-3;

            //reverse their order
            aux_s = s;
            aux_t = t;
            GIMS_LineSegment *auxls;
            int count = 0;
            while (count < event.ls->size / 2) {
                count++;
                auxls = **aux_t;
                **aux_t = **aux_s;
                **aux_s = auxls;
                aux_t--;
                aux_s++;
            }

            //ensure order. if a line segment intersects two overlaping line segments
            //(in the overlaping section), the reversing process might break the wanted order.
            sweepLineX += 1e-3;
            bool sorted = false;
            while (!sorted) {
                sorted = true;
                aux_s = s;
                aux_t = s;
                aux_t++;
                for (; aux_s != t && aux_s != T.end() && aux_t != T.end(); aux_s++, aux_t++) {
                    if (!BO_lscmp(*aux_s, *aux_t)) {
                        auxls = **aux_t;
                        **aux_t = **aux_s;
                        **aux_s = auxls;
                        sorted = false;
                    }
                }
            }
            sweepLineX -= 1e-3;

            //find the above and below line segments
            BO_lsset::iterator above = t;
            above++;
            BO_lsset::iterator below = s;
            if (below == T.begin())
                below = T.end();
            else
                below--;

            if (below != T.end()) {
                //add below, event.ls2 intersections
                int_p = report(intersections, **below, **s);
                if (int_p != NULL) {
                    newIntersectionBOEvent(eventQueue, int_p, **below, **s);
                }
            }

            if (above != T.end()) {
                //add event.ls1, above intersections
                int_p = report(intersections, **above, **t);
                if (int_p != NULL) {
                    newIntersectionBOEvent(eventQueue, int_p, **t, **above);
                }
            }
        }
    }
    return intersections;
}
