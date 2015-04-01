#include "BentleySolver.hpp"

double sweepLineX = 0;
double sweepLineY = 0;

double BentleySolver::getYatX(GIMS_LineSegment *l){

    if( l->p1->x == l->p2->x ){
        if( l->p1->x >= sweepLineX ){
            double miny = MIN(l->p1->y, l->p2->y),
                   maxy = MAX(l->p1->y, l->p2->y);
            return MIN( MAX(sweepLineY - 1e-3, miny), maxy );
        }else{
            return MAX(l->p1->y, l->p2->y);
        }
    }

    if( l->p1->y == l->p2->y )
        return l->p1->y;

    double m = ( l->p1->y - l->p2->y ) / ( l->p1->x - l->p2->x ),
           b = l->p1->y - m * l->p1->x;

    return m * sweepLineX + b;
}



int BentleySolver::cmp_linesegs(GIMS_LineSegment *a, GIMS_LineSegment *b){
    
    if( a->p1->x < b->p1->x )
        return 1;
    else if(a->p1->x == b->p1->x){
        if(a->p1->y < b->p1->y)
            return 1;
        else if(a->p1->y > b->p1->y)
            return -1;
    }else
        return -1;

    //at this point we know that a->p1 equals b->p1 
    if( a->p2->x < b->p2->x )
        return 1;
    else if(a->p2->x == b->p2->x){
        if(a->p2->y < b->p2->y)
            return 1;
        else if(a->p2->y > b->p2->y)
            return -1;
    }else
        return -1;

    //if this point is reached, the line segments are equal
    return 0;
}



bool BentleySolver::compare(Event a, Event b){
    if(a.pt->x < b.pt->x - ERR_MARGIN ){
        return true;
    
    }else if(a.pt->x < b.pt->x + ERR_MARGIN && a.pt->x > b.pt->x - ERR_MARGIN ){

        if( a.pt->y < b.pt->y - ERR_MARGIN )
            return true;
        else if( a.pt->y < b.pt->y + ERR_MARGIN && a.pt->y > b.pt->y - ERR_MARGIN ){

            if( a.type == 0 && b.type > 0 )
                return true;
            else if( b.type == 0 && a.type > 0 )
                return false;
            else if( a.type == 2 && b.type == 1 )
                return true;
            else if( b.type == 2 && a.type == 1 )
                return false;
        }else
            return false;
    }else
        return false;

    //at this point, we know that the events start at the same point
    //and take the same precedence. Therefore, the order is not important.
    //Never the less, we must assure a predictable ordering if the events are
    //in fact different.

    //We need to be careful defining "different" events at the same point. They
    //are different only if none of the linesegments of "a" is present in "b".
    for(int i=0; i<a.ls->size; i++){
        for(int j=0; j<b.ls->size; j++){
            if( *(a.ls->list[i]) == *(b.ls->list[j]) )
                return false;
        }
    }

    return cmp_linesegs(a.ls->list[0], b.ls->list[0]) > 0 ? true : false;
}



bool BentleySolver::lscmp(GIMS_LineSegment **ls1, GIMS_LineSegment **ls2){
    GIMS_LineSegment *a = *ls1,
                     *b = *ls2;

    double ay = getYatX(a),
           by = getYatX(b);

    if(ay < by){
        return true;
    }else if(ay == by){
        return cmp_linesegs(a, b) > 0 ? true : false;
    }

    return false;
}




GIMS_Point *BentleySolver::report(list<GIMS_Geometry *> &intersections, GIMS_LineSegment *a, GIMS_LineSegment *b){
    GIMS_Geometry *g = a->intersects(b);

    if( g == NULL ){
        return NULL;
    }

    if( g->type == LINESEGMENT ){
        if(a->id != b->id)
            intersections.push_back(g);
        return NULL;
    }

    if( ((GIMS_Point *)g)->x >= sweepLineX - 1e-3 )
        return (GIMS_Point *)g;
    return NULL;
}



bool BentleySolver::newIntersectionEvent(evset &eventQueue, GIMS_Point *int_p, GIMS_LineSegment *a, GIMS_LineSegment *b){

    GIMS_MultiLineSegment *mls = new GIMS_MultiLineSegment(2);
    mls->append(a); mls->append(b);

    Event e = {int_p, 2, mls};

    bool add_a = true, add_b = true;
    evset::iterator it = eventQueue.find(e);
    if( it != eventQueue.end() ){

        for(int i=0; i<it->ls->size; i++){
            if( *a == *(it->ls->list[i]) )
                add_a = false;
            if( *b == *(it->ls->list[i]) )
                add_b = false;
        }

        if(add_a) it->ls->append(a);
        if(add_b) it->ls->append(b);
        
        return true;

    }else{
        eventQueue.insert(e);
        return false;
    }
}



list<GIMS_Geometry *> BentleySolver::solve(GIMS_MultiLineString *A, GIMS_MultiLineString *B){

    list<GIMS_Geometry *> intersections;

    evset eventQueue(&compare);

    int nextId = 0;

    GIMS_MultiLineSegment *mls;
    GIMS_MultiLineString *src[2] = {A, B};

    for(GIMS_MultiLineString *aux : src){
        nextId++;
        for(int i=0; i<aux->size; i++){
            for(int j=0; j<aux->list[i]->size-1; j++){
                Event a, b;
                GIMS_LineSegment ls = aux->list[i]->getLineSegment(j);
                
                a.pt = ls.p1; b.pt = ls.p2;
                if( compare(a,b) ){
                    a.type = 0; b.type = 1;  
                }else{
                    a.type = 1; b.type = 0;
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
    lsset T(&lscmp);

    Event event;
    GIMS_Point *int_p;

    while( !eventQueue.empty() ){

        //fetch the next event from the queue
        event = *(eventQueue.begin());
        eventQueue.erase(eventQueue.begin());

        //update the sweeping lines, we need the vertical for vertical line segments.
        sweepLineX = event.pt->x;
        sweepLineY = event.pt->y;

        //the event refers to a left endpoint
        if(event.type == 0){
            
            GIMS_LineSegment *ls = event.ls->list[0];

            sweepLineX -= 1e-3;
            pair<lsset::iterator, bool> r = T.insert(event.ls->list);
            sweepLineX += 1e-3;

            lsset::iterator above = r.first; above++;
            lsset::iterator below = r.first;
            if( below == T.begin() )
                below = T.end();
            else
                below--;

            if(above != T.end()){
                int_p = report(intersections, ls, **above);
                if( int_p != NULL )
                    newIntersectionEvent(eventQueue, int_p, ls, **above);
            }

            if(below != T.end()){
                int_p = report(intersections, ls, **below);
                if( int_p != NULL )
                    newIntersectionEvent(eventQueue, int_p, **below, ls);
            }


        //the event refers to a right endpoint
        }else if(event.type == 1){

            GIMS_LineSegment *ls = event.ls->list[0];

            sweepLineX += 1e-3;
            lsset::iterator it = T.find(&ls);
            lsset::iterator above = it; above++;
            lsset::iterator below = it;
            if( below == T.begin() )
                below = T.end();
            else
                below--;
            T.erase(it);
            sweepLineX -= 1e-3;

            if(above != T.end() && below != T.end()){
                int_p = report(intersections, **above, **below);
                if( int_p != NULL ){
                    newIntersectionEvent( eventQueue, int_p, **below, **above );
                }
            }

            //free up the memory allocated for the line segment
            event.ls->deepDelete();


        //the event refers to an intersection point
        }else if( event.type == 2 ){

            int previd = event.ls->list[0]->id;
            for(int i=1; i<event.ls->size; i++){
                if( event.ls->list[i]->id != previd ){
                    intersections.push_back(event.pt->clone());
                    break;
                }
            }

            sweepLineX -= 1e-3;
            //swap positions of s and t in T
            lsset::iterator s = T.end(),
                            t = T.begin(),
                            aux, aux_s, aux_t;
            s--;

            //find the first and last line segments
            for(int i=0; i<event.ls->size; i++){
                aux = T.find(&(event.ls->list[i]));

                if( lscmp(*aux, *s) )
                    s = aux;
                if( lscmp(*t, *aux) )
                    t = aux;
            }
            sweepLineX += 1e-3;

            //reverse their order
            aux_s = s; aux_t = t;
            GIMS_LineSegment *auxls;
            int count = 0;
            while(count < event.ls->size/2){
                count++;
                auxls = **aux_t;
                **aux_t = **aux_s;
                **aux_s = auxls;
                aux_t--; aux_s++;
            }

            //ensure order. if a line segment intersects two overlaping line segments 
            //(in the overlaping section), the reversing process might break the wanted order. 
            sweepLineX += 1e-3;
            bool sorted = false;
            while(!sorted){
                sorted = true;
                aux_s = s;
                aux_t = s; aux_t++;
                for(; aux_s != t; aux_s++,aux_t++){
                    if( !lscmp(*aux_s, *aux_t) ){
                        auxls = **aux_t;
                        **aux_t = **aux_s;
                        **aux_s = auxls;
                        sorted = false;
                    }
                }
            }
            sweepLineX -= 1e-3;

            //find the above and below line segments
            lsset::iterator above = t; above++;
            lsset::iterator below = s;
            if( below == T.begin() )
                below = T.end();
            else
                below--;

            if( below != T.end() ){
                //add below, event.ls2 intersections
                int_p = report(intersections, **below, **s);
                if(int_p != NULL){
                    newIntersectionEvent(eventQueue, int_p, **below, **s);
                }
            }
            
            if( above != T.end() ){
                //add event.ls1, above intersections
                int_p = report(intersections, **above, **t);
                if(int_p != NULL){
                    newIntersectionEvent(eventQueue, int_p, **t, **above);
                }

            }
        }
    }
    return intersections;
}