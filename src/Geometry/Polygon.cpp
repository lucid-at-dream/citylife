#include "Geometry.hpp"

void GIMS_Polygon::deepDelete(){
    if(externalRing != NULL){
        this->externalRing->deepDelete();
        this->externalRing = NULL;
    }
    if(internalRings != NULL){
        this->internalRings->deepDelete();
        this->internalRings = NULL;
    }
    delete this;
}

void GIMS_Polygon::deleteClipped(){
    if(this->externalRing != NULL){
        this->externalRing->deleteClipped();
        this->externalRing = NULL;
    }
    if(this->internalRings != NULL){
        this->internalRings->deleteClipped();
        this->internalRings = NULL;
    }
    delete this;
}

GIMS_BoundingBox *GIMS_Polygon::getExtent(){
    double maxx=-1e100, maxy=-1e100, minx=1e100, miny=1e100;
    
    for(int i=0; i<this->externalRing->list[0]->size; i++){
        if(this->externalRing->list[0]->list[i]->x > maxx)
            maxx = this->externalRing->list[0]->list[i]->x;
        if(this->externalRing->list[0]->list[i]->y > maxy)
            maxy = this->externalRing->list[0]->list[i]->y;
        if(this->externalRing->list[0]->list[i]->x < minx)
            minx = this->externalRing->list[0]->list[i]->x;
        if(this->externalRing->list[0]->list[i]->y < miny)
            miny = this->externalRing->list[0]->list[i]->y;
    }

    return new GIMS_BoundingBox(new GIMS_Point(minx, miny), new GIMS_Point(maxx, maxy));
}

string GIMS_Polygon::toWkt(){
    string wkt = string("POLYGON(");
    char buff[100];
    for(int i=0; i<this->externalRing->list[0]->size; i++){
        if(i==0)
            wkt += "(";
        sprintf(buff, "%lf %lf", this->externalRing->list[0]->list[i]->x, this->externalRing->list[0]->list[i]->y);
        wkt += string(buff) + ( i < this->externalRing->list[0]->size - 1 ? string(",") : string(")") );
    }

    if( this->internalRings == NULL){
        wkt += ")";
        return wkt;
    }else
        wkt += ",";

    for(int i=0; i<this->internalRings->size; i++){
        for(int j=0; j<this->internalRings->list[i]->size; j++){
            if(j==0)
                wkt += "(";
            sprintf(buff, "%lf %lf", this->internalRings->list[i]->list[j]->x, this->internalRings->list[i]->list[j]->y);
            wkt += string(buff) + ( j < this->internalRings->list[i]->size - 1 ? string(",") : string(")") );
        }
        if( i < this->internalRings->size - 1 )
            wkt += ",";
        else
            wkt += ")";
    }

    return wkt;
}

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
    if( this->externalRing )
        delete this->externalRing;
    if( this->internalRings )
        delete this->internalRings;
}




void GIMS_MultiPolygon::deepDelete(){
    if( this->list != NULL )
        for(int i=0; i<this->size; i++)
            this->list[i]->deepDelete();
    delete this;
}

void GIMS_MultiPolygon::deleteClipped(){
    for(int i=0; i<this->size; i++){
        this->list[i]->deleteClipped();
    }
    delete this;
}

void GIMS_MultiPolygon::append(GIMS_Polygon *p){
    this->size++;
    if(this->size > this->allocatedSize){
        this->list = (GIMS_Polygon **)realloc(this->list, this->size * sizeof(GIMS_Polygon *));
        this->allocatedSize = size;
    }
    this->list[size-1] = p;
}

string GIMS_MultiPolygon::toWkt(){
    string wkt = string("MULTIPOLYGON(");
    char buff[100];
    for(int k=0; k<this->size; k++){
        wkt += "(";
        GIMS_Polygon *pol = this->list[k];
        for(int i=0; i<pol->externalRing->list[0]->size; i++){
            if(i==0)
                wkt += "(";
            sprintf(buff, "%lf %lf", pol->externalRing->list[0]->list[i]->x, pol->externalRing->list[0]->list[i]->y);
            wkt += string(buff) + ( i < pol->externalRing->list[0]->size - 1 ? string(",") : string(")") );
        }

        if( pol->internalRings == NULL){
            wkt += ")";
        }else{
            wkt += ",";
            for(int i=0; i<pol->internalRings->size; i++){
                for(int j=0; j<pol->internalRings->list[i]->size; j++){
                    if(j==0)
                        wkt += "(";
                    sprintf(buff, "%lf %lf", pol->internalRings->list[i]->list[j]->x, pol->internalRings->list[i]->list[j]->y);
                    wkt += string(buff) + ( j < pol->internalRings->list[i]->size - 1 ? string(",") : string(")") );
                }
                wkt += i < pol->internalRings->size - 1 ? "," : ")";
            }
        }

        wkt += k < this->size - 1 ? "," : ")";

    }

    return wkt;
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
    if(this->list)
        free(this->list);
}

