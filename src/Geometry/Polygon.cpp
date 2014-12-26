#include "Geometry.hpp"

GIMS_Polygon *GIMS_Polygon::clone(){    
    return new GIMS_Polygon(this->externalRing->clone(),this->internalRings->clone());
}

GIMS_Geometry *GIMS_Polygon::clipToBox(GIMS_BoundingBox *box){
    GIMS_MultiLineString *exterior = (GIMS_MultiLineString *)(this->externalRing->clipToBox(box));

    GIMS_MultiLineString **interior = (GIMS_MultiLineString *)(this->internalRings->clipToBox(box));
    
    if(exterior != NULL || interior != NULL)
        return new GIMS_Polygon( exterior, interior );
    else
        return NULL;
}

void appendInternalRing(GIMS_LineString *ir){
    this->internalRings->append(ir);
}

GIMS_Polygon::GIMS_Polygon(GIMS_LineString *externalRing, GIMS_MultiLineString *internalRings){
    this->type = POLYGON;
    this->externalRing = externalRing;
    this->internalRings = internalRings;
}

GIMS_Polygon::GIMS_Polygon(int ext_alloc, int int_alloc){
    this->type = POLYGON;
    this->externalRing = new GIMS_LineString(ext_alloc);
    this->internalRIngs = new GIMS_MultiLineString(int_alloc);
}

GIMS_Polygon::GIMS_Polygon(){
    this->type = POLYGON;
    this->externalRing = this->internalRings = NULL;
}

GIMS_Polygon::~GIMS_Polygon(){
    delete this->externalRing;
    delete this->internalRings;
}
