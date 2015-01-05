#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "PGConnection.h"
#include "Connect2Postgis.h"
#include "Geometry.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"

using namespace PMQUADTREE;
using namespace GIMS_GEOMETRY;

int total = 0;
unsigned long int incId = 1;

GIMS_Geometry *retrieveFeature ( OGRFeature *feature );

int main (int argc, char *argv[]) {

    Geometry::Connect2Postgis getGeom;
    OGRLayer *layer = getGeom.GetLayerByName ("planet_osm_polygon");
    
    OGREnvelope *envelope = new OGREnvelope;
    if ( layer->GetExtent ( envelope, FALSE ) != OGRERR_NONE ) {
        perror ("could not retrieve layer envelope");
        exit (-1);
    }
    double lenx = envelope->MaxX - envelope->MinX,
           leny = envelope->MaxY - envelope->MinY,
           len  = lenx > leny ? lenx : leny;

    PMQuadTree *tree = 
        new PMQuadTree( new GIMS_BoundingBox(
            new GIMS_Point (envelope->MinX, envelope->MinY),
            new GIMS_Point (envelope->MinX + len, envelope->MinY + len)
        ));
    
    OGRFeature *feature;
    GIMS_Geometry* query;
    GIMS_Geometry* aux;
    int count = 0;
    while ( (feature = layer->GetNextFeature() ) != NULL) {
    
        /*
        if ( feature->GetFieldAsDouble(feature->GetFieldIndex("way_area")) < 10000 ) {
            delete feature;
            continue;
        }
        */
        //printf("%d\n", count);
        aux = retrieveFeature (feature);
        
        if(aux != NULL){
            aux->id = incId++;
            tree->insert ( aux );
            count++;
        }
        delete feature;

        if(count >= 50)
            break;

    }
    printf("inserted %d points.\n", total);
    
    //tree->query = query;
    renderer = new DebRenderer();
    renderer->setScale(400.0/lenx, -400.0/leny);
    renderer->setTranslation( -envelope->MinX, -envelope->MaxY );
    renderer->renderCallback = tree;
    //renderer->renderSvg("outtree.svg", 400, 400);
    renderer->mainloop(argc, argv);

    delete layer;
    
    return 0;
}

GIMS_Geometry *retrieveFeature ( OGRFeature *feature ) {

    OGRGeometry *geometry = feature->GetGeometryRef();
    
    if (geometry->getGeometryType() == wkbPoint) { //wkbMultiPoint
        total++;
        OGRPoint *f_point = (OGRPoint *)geometry;
        GIMS_Point *point = new GIMS_Point(f_point->getX(), f_point->getY());
        return point;

    } else if (geometry->getGeometryType() == wkbLineString) { //wkbMultiLineString 

        OGRLineString *f_lineString = ( (OGRLineString *) geometry);
        int N = f_lineString->getNumPoints();
        total += N;
        GIMS_LineString *lineString = new GIMS_LineString(N);

        OGRPoint *tmp = new OGRPoint;
        for (int i = 0; i < N; i++) {
            f_lineString->getPoint (i, tmp);
            lineString->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
        }
        
        delete tmp;
        return lineString;
        
    } else if (geometry->getGeometryType() == wkbPolygon) { //wkbMultiPolygon
 
        OGRLinearRing *extRing = ( (OGRPolygon *) geometry)->getExteriorRing();
        int N = extRing->getNumPoints();
        GIMS_Ring *exteriorRing = new GIMS_Ring(N);

        total += N;
        OGRPoint *tmp = new OGRPoint;
        for (int i = 0; i < N; i++) {
            extRing->getPoint (i, tmp);
            exteriorRing->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
        }
        
        int M = ( (OGRPolygon *) geometry)->getNumInteriorRings();
        GIMS_Polygon *polygon = new GIMS_Polygon(1,M);
        polygon->appendExternalRing(exteriorRing);

        for( int k=0; k<M; k++ ){ 
            OGRLinearRing *f_intRing = ( (OGRPolygon *) geometry)->getInteriorRing(k);
            N = f_intRing->getNumPoints();
            total += N;
            GIMS_Ring *intRing = new GIMS_Ring(N);

            for (int i = 0; i < N; i++) {
                f_intRing->getPoint (i, tmp);
                intRing->appendPoint(new GIMS_Point(tmp->getX(), tmp->getY()));
            }
            if( intRing->list[0]->equals(intRing->list[intRing->size-1]) )
                printf("wololo\n");

            polygon->appendInternalRing(intRing);
        }

        delete tmp;
        return polygon;
    } else {
        perror ("unsupported type of geometry detected.");
        return NULL;
    }

}
