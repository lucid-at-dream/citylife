#include "Geometry.hpp"

GIMS_Polygon *GIMS_Polygon::clone(){    
    GIMS_Polygon *fresh = new GIMS_Polygon(this->externalRing->clone(),this->internalRings->clone());
    fresh->id = this->id;
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
    this->type = POLYGON;
    this->externalRing = new GIMS_MultiLineString(ext_alloc);
    this->internalRings = new GIMS_MultiLineString(int_alloc);
}

GIMS_Polygon::GIMS_Polygon(){
    this->type = POLYGON;
    this->externalRing = this->internalRings = NULL;
}

GIMS_Polygon::~GIMS_Polygon(){
    delete this->externalRing;
    delete this->internalRings;
}
