#include "GeometryList.hpp"

/*
-->GIMSGeometryList
*/
GIMSGeometryList *GIMSGeometryList::clone () {
    TODO(Implement clone function on the geometry list)
    return NULL;
}

GIMSGeometry *GIMSGeometryList::clipToBox ( GIMSBoundingBox *box ){
    GIMSGeometryList *clipped = NULL;

    for( std::list<GIMSGeometry *>::iterator it = this->list->begin();
         it != this->list->end(); it++                                 ) {
        GIMSGeometry *g = (*it)->clipToBox(box);

        if( g != NULL ){
            if( clipped == NULL ){
                clipped = new GIMSGeometryList;
            }
            clipped->list->push_back(g);
        }
    }
    if(clipped != NULL)
        clipped->id = this->id;
    return clipped;
}

GIMSGeometryList::GIMSGeometryList(){
    this->type = MIXEDLIST;
    this->list = new std::list<GIMSGeometry *>();
    this->renderCount = 0;
}

GIMSGeometryList::~GIMSGeometryList(){
    delete this->list;
}
/*
GIMSGeometryList<--
*/
