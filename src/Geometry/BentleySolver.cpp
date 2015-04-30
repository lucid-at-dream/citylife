#include "BentleySolver.hpp"

#define _BS_PRECISION 1000000
#define THRESHOLD 20

bool cmp(GIMS_LineSegment a, GIMS_LineSegment b){
    return a.osm_id < b.osm_id;
}

bool compare(Event a, Event b){
    if(a.pt->x < b.pt->x - ERR_MARGIN ){
        return false;
    }else if(a.pt->x < b.pt->x + ERR_MARGIN && a.pt->x > b.pt->x - ERR_MARGIN ){

        if( a.pt->y < b.pt->y - ERR_MARGIN )
            return false;
        else if( a.pt->y < b.pt->y + ERR_MARGIN && a.pt->y > b.pt->y - ERR_MARGIN ){
            
            //if x and y are equal, differentiate by event type
            if( a.type < b.type )
                return false;
            else if( b.type < a.type )
                return true;

            //if both x and y are equal and the event type is equal
            //differentiate by the given ID
            else if( a.ls.id < b.ls.id )
                return false;
            
            //finally, we must distinguish line segments that start at the same point and
            //are in the same set.
            else
                return lscmp(a.ls, b.ls);

        }else
            return true;
    }else 
        return true;
}


bool lscmp(GIMS_LineSegment &a, GIMS_LineSegment &b){

    if( a.p1->x < b.p1->x )
        return true;
    else if(a.p1->x == b.p1->x){
        if(a.p1->y < b.p1->y)
            return true;
        else if(a.p1->y > b.p1->y)
            return false;
    }else
        return false;

    //at this point we know that a.p1 equals b.p1 
    if( a.p2->x < b.p2->x )
        return true;
    else if(a.p2->x == b.p2->x){
        if(a.p2->y < b.p2->y)
            return true;
        else if(a.p2->y > b.p2->y)
            return false;
    }else
        return false;

    //if this point is reached, the line segments are equal
    return false;
}

list<GIMS_Geometry *> BentleySolver::bruteforce(GIMS_MultiLineString *A, GIMS_MultiLineString *B){
    list<GIMS_Geometry *> intersections;
    for(int i=0; i<A->size; i++){
        for(int j=0; j<A->list[i]->size - 1; j++){
            
            for(int k=0; k<B->size; k++){
                for(int m=0; m<B->list[k]->size - 1; m++){
                    GIMS_LineSegment _a = A->list[i]->getLineSegment(j);
                    GIMS_LineSegment _b = B->list[k]->getLineSegment(m);

                    GIMS_Geometry * aux = _a.intersects(&_b);

                    if(aux != NULL)
                        intersections.push_back(aux);
                }
            }
        }
    }
    return intersections;
}

double BentleySolver::inputMLS( evset &eventQueue, GIMS_MultiLineString *mls, int id){
    double max_x = -1e100;
    int count = 1;
    for(int i=0; i<mls->size; i++){
        for(int j=0; j<mls->list[i]->size-1; j++){
            Event a, b;
            GIMS_LineSegment ls = mls->list[i]->getLineSegment(j);
            ls.id = id;
            ls.osm_id = count++;

            a.ls = ls;
            b.ls = ls;
            a.type = 0; b.type = 1;
            
            if( ls.p1->x < ls.p2->x ){
                a.pt = ls.p1; b.pt = ls.p2;
            }else if(ls.p1->x > ls.p2->x){
                a.pt = ls.p2; b.pt = ls.p1;
            }else{
                if( ls.p1->y < ls.p2->y ){
                    a.pt = ls.p1; b.pt = ls.p2;
                }else if(ls.p1->y > ls.p2->y){
                    a.pt = ls.p2; b.pt = ls.p1;
                }else{
                    cerr << "degenerate line segment (it's a point)" << endl;
                    continue;
                }
            }

            if( ls.p1->x > max_x )
                max_x = ls.p1->x;
            if( ls.p2->x > max_x )
                max_x = ls.p2->x;

            eventQueue.push(a);
            eventQueue.push(b);
        }
    }
    return max_x;
}


list<GIMS_Geometry *> BentleySolver::solve(GIMS_MultiLineString *A, GIMS_MultiLineString *B){

    bool disjoint = true;
    for( int i=0; disjoint && i<A->size; i++ ){
        for( int j=0; disjoint && j<B->size; j++ ){
            if( !(A->list[i]->bbox.isDisjoint( &(B->list[j]->bbox) )) )
                disjoint = false;
        }
    }

    if(disjoint)
        return list<GIMS_Geometry *>();

    if( A->getPointCount() < THRESHOLD || B->getPointCount() < THRESHOLD ){
        return this->bruteforce(A, B);
    }else{
        return this->linesweep(A, B);
    }
}

GIMS_Point *getrp(GIMS_LineSegment &ls){
    if(ls.p1->x < ls.p2->x)
        return ls.p2;
    else if( ls.p1->x > ls.p2->x )
        return ls.p1;
    else{
        if(ls.p1->y < ls.p2->y)
            return ls.p2;
        else if( ls.p1->y > ls.p2->y )
            return ls.p1;
    }
    return 0;
}

bool fktp_cmp(GIMS_LineSegment &a, GIMS_LineSegment &b){
    GIMS_Point *arp = getrp(a),
               *brp = getrp(b);

    if(arp->x < brp->x)
        return true;
    else if( arp->x > brp->x )
        return false;
    else{
        if(arp->y < brp->y)
            return true;
        else if( arp->y > brp->y )
            return false;
    }
    return false;
}

void insert(lsset &l, GIMS_LineSegment &ls){
    for(lsset::iterator it = l.begin(); it != l.end(); it++){
        if( !fktp_cmp(*it, ls) ){
            l.insert(it, ls);
            return;
        }
    }
    l.push_back(ls);
}

list<GIMS_Geometry *> BentleySolver::linesweep(GIMS_MultiLineString *A, GIMS_MultiLineString *B){

    list<GIMS_Geometry *> intersections;
    evset eventQueue(&compare);

    double last_x_A = this->inputMLS(eventQueue, A, 1);
    double last_x_B = this->inputMLS(eventQueue, B, 2);
    double last_x = MIN(last_x_A, last_x_B); //after this value of X, no intersections happen.
    
    lsset red, blue;

    Event event;
    while( !eventQueue.empty() ){
        
        event = eventQueue.top();
        eventQueue.pop();
        if( event.pt->x > last_x )
            break;

        if( event.type == 0 ){
            if( event.ls.id == 1 ){ //red
                for(lsset::iterator it = blue.begin(); it != blue.end(); it++){
                    GIMS_Geometry *g = event.ls.intersects( (&(*it)) );
                    if( g != NULL )
                        intersections.push_back(g);
                }
                insert(red, event.ls);
            }else{ //blue
                for(lsset::iterator it = red.begin(); it != red.end(); it++){
                    GIMS_Geometry *g = event.ls.intersects( (&(*it)) );
                    if( g != NULL )
                        intersections.push_back(g);
                }
                insert(blue, event.ls);
            }
        }

        else if( event.type == 1 ){
            if( event.ls.id == 1 ){ //red
                for(lsset::iterator it = red.begin(); it != red.end(); it++){
                    if( lscmp( *it, event.ls) == 0 ){
                        red.erase(it);
                        break;
                    }
                }
            }else{ //blue
                for(lsset::iterator it = blue.begin(); it != blue.end(); it++){
                    if( lscmp( *it, event.ls) == 0 ){
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
