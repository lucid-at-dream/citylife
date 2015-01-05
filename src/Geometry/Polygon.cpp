#include "Geometry.hpp"

GIMS_Polygon *GIMS_Polygon::clone(){    
    GIMS_Polygon *fresh = new GIMS_Polygon(this->externalRing->clone(),this->internalRings->clone());
    fresh->id = this->id;
    return fresh;
}

GIMS_Geometry *GIMS_Polygon::clipToBox(GIMS_BoundingBox *box){
    GIMS_MultiLineString *exterior = NULL,
                         *interior = NULL;
    
    if(this->externalRing != NULL)
        exterior = (GIMS_MultiLineString *)(this->externalRing->clipToBox(box));

    if(this->internalRings != NULL)
        interior = (GIMS_MultiLineString *)(this->internalRings->clipToBox(box));

    if(exterior != NULL || interior != NULL){
        GIMS_Polygon *clipped = new GIMS_Polygon( exterior, interior );
        clipped->id = this->id;
        return clipped;
    }else
        return NULL;
}

void GIMS_Polygon::appendExternalRing(GIMS_LineString *er){
    if(this->externalRing == NULL)
        this->externalRing = new GIMS_MultiLineString(1);
    this->externalRing->append(er);
}

void GIMS_Polygon::appendInternalRing(GIMS_LineString *ir){
    if(this->internalRings == NULL)
        this->internalRings = new GIMS_MultiLineString(1);
    this->internalRings->append(ir);
}

GIMS_Polygon::GIMS_Polygon(GIMS_MultiLineString *externalRing, GIMS_MultiLineString *internalRings){
    this->type = POLYGON;
    this->externalRing = externalRing;
    this->internalRings = internalRings;
}

GIMS_Polygon::GIMS_Polygon(int ext_alloc, int int_alloc){
    this->id = 0;
    this->type = POLYGON;
    this->externalRing = new GIMS_MultiLineString(ext_alloc);
    this->internalRings = new GIMS_MultiLineString(int_alloc);
}

GIMS_Polygon::GIMS_Polygon(){
    this->id = 0;
    this->type = POLYGON;
    this->externalRing = this->internalRings = NULL;
}

GIMS_Polygon::~GIMS_Polygon(){
    delete this->externalRing;
    delete this->internalRings;
}






void GIMS_MultiPolygon::append(GIMS_Polygon *p){
    this->size++;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_Polygon **)realloc(this->list, this->size * sizeof(GIMS_Polygon *));
        this->allocatedSize = size;
    }
    this->list[size-1] = p;
}

GIMS_MultiPolygon *GIMS_MultiPolygon::clone(){
    GIMS_MultiPolygon *fresh = new GIMS_MultiPolygon(this->allocatedSize);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_Polygon *));
    fresh->size = this->size;
    fresh->id = this->id;
    return fresh;
}

GIMS_Geometry *GIMS_MultiPolygon::clipToBox(GIMS_BoundingBox *box){

    if(this->list == NULL) return NULL;

    GIMS_MultiPolygon *clipped = NULL;

    for(int i=0; i<this->size; i++){
        
        GIMS_Polygon *partial = (GIMS_Polygon *)(this->list[i]->clipToBox(box));
        if(partial != NULL){
            if(clipped == NULL){
                clipped = new GIMS_MultiPolygon(1);
                clipped->id = this->id;
            }
            clipped->append(partial);
        }
    }
    return clipped;
}

GIMS_MultiPolygon::GIMS_MultiPolygon(){
    this->id = 0;
    this->type = MULTIPOLYGON;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
}

GIMS_MultiPolygon::GIMS_MultiPolygon(int size){
    this->type = MULTIPOLYGON;
    this->id = 0;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_Polygon **)malloc(size * sizeof(GIMS_Polygon *));
}

GIMS_MultiPolygon::~GIMS_MultiPolygon(){
    free(this->list);
}

