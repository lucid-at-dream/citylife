#include "Polygon.hpp"

GIMS_Polygon *GIMS_Polygon::clone(){    
    GIMS_LineString **ir_fresh = (GIMS_LineString **)malloc(this->ir_allocatedSize * sizeof(GIMS_LineString *));
    memcpy(ir_fresh, this->internalRings, this->ir_size);
    GIMS_Polygon *fresh = new GIMS_Polygon(this->externalRing->clone(),ir_fresh);

    fresh->ir_size = this->ir_size;
    fresh->ir_allocatedSize = this->ir_allocatedSize;
    
    return fresh;
}

GIMS_Geometry *GIMS_Polygon::clipToBox(GIMS_BoundingBox *box){
    GIMS_LineString *exterior = (GIMS_LineString *)(this->externalRing->clipToBox(box));

    GIMS_LineString **interior = NULL; int count = 0;
    for(int i=0; i<this->ir_size; i++){
        GIMS_LineString *aux = (GIMS_LineString *)(this->internalRings[i]->clipToBox(box));
        if( aux != NULL ){
            count++;
            interior = (GIMS_LineString **)realloc(interior, count * sizeof(GIMS_LineString *));
            interior[count-1] = aux;
        }
    }
    
    if(exterior != NULL || interior != NULL)
        return new GIMS_Polygon( exterior, interior );
    else
        return NULL;
}

void appendInternalRing(GIMS_Ring *ir){
    this->ir_size++;
    if(this->ir_size > this->ir_allocatedSize){
        this->internalRings = (GIMS_LineString **)realloc(this->internalRings, this->ir_size * sizeof(GIMS_LineString *));
        this->ir_allocatedSize = this->ir_size;
    }
    this->internalRings[ir_size-1] = ir;
}

GIMS_Polygon::GIMS_Polygon( GIMS_LineString *externalRing, GIMS_LineString **internalRings, int ir_size, int ir_allocatedSize ){
    this->type = POLYGON;
    this->externalRing = externalRing;
    this->internalRings = internalRings;
    this->ir_size = ir_size;
    this->ir_allocatedSize = ir_allocatedSize;
}

GIMS_Polygon::GIMS_Polygon(){
    this->type = POLYGON;
    this->externalRing = this->internalRings = NULL;
    this->ir_size = this->ir_allocatedSize = 0;
}

GIMS_Polygon::~GIMS_Polygon(){
    delete this->externalRing;
    delete this->internalRings;
}
