#include "Polygon.hpp"

GIMSPolygon *GIMSPolygon::clone(){
    TODO(Implement clone function for polygons);
    perror("called an unimplemented function (GIMSPolygon__clone)");
    exit(-1);
}

GIMSGeometry *GIMSPolygon::clipToBox(GIMSBoundingBox *box){
    GIMSGeometryList *exterior = (GIMSGeometryList *)(this->externalRing->clipToBox(box));
    GIMSGeometryList *interior = (GIMSGeometryList *)(this->internalRings->clipToBox(box));

    if(exterior != NULL || interior != NULL ){
        GIMSGeometry *g = new GIMSPolygon( exterior, interior );
        g->id = this->id;
        return g;
    }else
        return NULL;
}

GIMSPolygon::GIMSPolygon( GIMSGeometryList *externalRing, GIMSGeometryList *internalRings ){
    this->type = POLYGON;
    this->renderCount = 0;
    if(externalRing != NULL)
        this->externalRing = externalRing;
    else
        this->externalRing = new GIMSGeometryList();
    if(internalRings != NULL)
        this->internalRings = internalRings;
    else
        this->internalRings = new GIMSGeometryList();
}

GIMSPolygon::~GIMSPolygon(){
    delete this->externalRing;
    delete this->internalRings;
}
