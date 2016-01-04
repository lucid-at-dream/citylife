#include "DCEL.hpp"

vertex::vertex(){
    this->data = 0;
}

halfedge::halfedge(){
    this->data = 0;
    this->count = 0;
}

face::face(){
    this->data = 0;
}

void DCEL::addVertex(vertex *item){
    this->vertexes.insert(item);
}

vertex *DCEL::findVertex(GIMS_Point *p){
    vertex v;
    v.pt = p;
    vertexlist::iterator it = vertexes.find(&v);
    if( it != vertexes.end() )
        return *it;
    return NULL;
}

vertex *DCEL::findVertex(vertex *v){
    vertexlist::iterator it = vertexes.find(v);
    if( it != vertexes.end() )
        return *it;
    return NULL;
}

void DCEL::addHalfedge(halfedge *item){
    this->halfedges.insert(item);
}

halfedge *DCEL::findHalfedge(halfedge *h){
    halfedgelist::iterator it = halfedges.find(h);
    if( it != halfedges.end() )
        return *it;
    return NULL;
}

void DCEL::addFace(face *item){
    this->faces.push_back(item);
}

void DCEL::calculateFaces(){
    /*calculating faces is all about finding cycles in the graph, which is what we're doing here.*/
    for(halfedgelist::iterator it = halfedges.begin(); it != halfedges.end(); it++){

        halfedge *edge = *it;

        /*if count is different from 0, then the edge has already been analyzed.*/
        if( edge->count > 0 )
            continue;
        edge->count += 1;

        /*does this edge or the ones that follow it belong to a cycle?*/
        halfedge *auxedge = edge->next;
        while( auxedge != NULL && auxedge->count == 0 ){
            auxedge->count += 1;
            auxedge = auxedge->next;
        }

        if( auxedge == NULL )
            continue;

        /* At this point, if auxedge differs from NULL it belongs to a cycle. Therefore, since
         * we're now sure there is a cycle, we will proceed to identifying all nodes in 
         * the cycle and assigning them with a common face.*/
        face *f = new face();
        f->boundary = auxedge;
        this->addFace(f);

        edge = auxedge;
        auxedge = edge->next;
        while(auxedge != edge){
            auxedge->left = f;
            auxedge = auxedge->next;
        }
    }
}

bool sort_clockwise_cmp_(const halfedge *a, const halfedge *b){
    
    double v1_x = a->twin->tail->pt->x - a->tail->pt->x,
           v1_y = a->twin->tail->pt->y - a->tail->pt->y,
           v2_x = b->twin->tail->pt->x - b->tail->pt->x,
           v2_y = b->twin->tail->pt->y - b->tail->pt->y;

    double angle_a = atan2( v1_y, v1_x ),
           angle_b = atan2( v2_y, v2_x );

    return angle_a < angle_b;
}
