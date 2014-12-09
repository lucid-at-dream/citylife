#include "EdgeList.hpp"

GIMSEdgeList::GIMSEdgeList (int size){
    this->type = EDGELIST;
    this->list = (GIMSPoint **)malloc(sizeof(GIMSPoint *) * size);
    this->allocatedSize = size;
    this->size = 0;
}

GIMSEdgeList::GIMSEdgeList (){
    this->type = EDGELIST;
    this->list = NULL;
    this->allocatedSize = 0;
    this->size = 0;
}

GIMSEdgeList::~GIMSEdgeList (){
    if(this->list != NULL)
        free(this->list);
}

GIMSEdgeList *GIMSEdgeList::clone (){
    GIMSEdgeList *newList = new GIMSEdgeList(this->size);
    memcpy(newList->list, this->list, this->size*sizeof(GIMSPoint *));
    return newList;
}

GIMSGeometry *GIMSEdgeList::clipToBox (GIMSBoundingBox *box){

    GIMSGeometryList *clipped = new GIMSGeometryList();

    GIMSEdgeList *el = new GIMSEdgeList();
    for( int k = 0; k < this->size; k++ ){
        GIMSEdge e = this->getEdge(k);
        if( e.clipToBox(box) != NULL ){
            if(el == NULL){
                el = new GIMSEdgeList(2);
                el->appendPoint(e.p1);
            }
            el->appendPoint(e.p2);
        }else{
            if(el != NULL)
                clipped->list->push_back(el);
            el = NULL;
        }
    }

    return clipped;
}

GIMSEdge GIMSEdgeList::getEdge (int index){
    GIMSEdge edge = GIMSEdge( this->list[index%this->size],
                              this->list[(index+1)%this->size]);
    return edge;
}

void GIMSEdgeList::appendPoint(GIMSPoint *p){
    this->size += 1;

    if( this->size > this->allocatedSize ){
        int deficit = this->size - this->allocatedSize;
        this->list = (GIMSPoint **)realloc(this->list, 
                                           sizeof(GIMSPoint *) * (this->allocatedSize + deficit));
    }

    this->list[this->size-1] = p;
}

void GIMSEdgeList::appendPoint(double x, double y){
    this->size += 1;

    if( this->size > this->allocatedSize ){
        int deficit = this->size - this->allocatedSize;
        this->list = (GIMSPoint **)realloc(this->list, 
                                           sizeof(GIMSPoint *) * (this->allocatedSize + deficit));
    }

    this->list[this->size-1] = new GIMSPoint(x,y);
}
