#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include "PGConnection.h"
#include "Connect2Postgis.h"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"
#define INTERSECTION_COLOR "#D49"

using namespace PMQUADTREE;
using namespace GIMSGEOMETRY;

int total = 0;

GIMSGeometry *retrieveFeature ( OGRFeature *feature );
void dumpList (std::list<GIMSGeometry *> *list);

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
    
    PMQuadTree *tree = new PMQuadTree( new GIMSBoundingBox(
                                           new GIMSPoint (envelope->MinX, envelope->MinY),
                                           new GIMSPoint (envelope->MinX + len, envelope->MinY + len) ) );
    
    OGRFeature *feature;
    GIMSGeometry* query;
    int count = 0;
    while ( (feature = layer->GetNextFeature() ) != NULL) {
    
        /*
        if ( strcmp (feature->GetFieldAsString (feature->GetFieldIndex ("highway") ), "cycleway") != 0 ) {
            delete feature;
            continue;
        }*/
        
        printf("%d\n", count);

        if(count == 0)
            tree->insert ( query = retrieveFeature (feature) );
        else
            tree->insert ( retrieveFeature (feature) );
        count++;
        delete feature;
        if(count > 250)
            break;
    }
    tree->query = query;
    printf("inserted %d edges.\n", total);
    
    renderer = new DebRenderer();
    renderer->renderCallback = tree;
    //renderer->renderSvg("outtree.svg", 400, 400);
    renderer->mainloop(argc, argv);

    delete layer;
    
    return 0;
}

void dumpList (std::list<GIMSGeometry *> *list) {
    std::list<GIMSGeometry *>::iterator it = list->begin();
    
    for (; it != list->end(); it++) {
        printf ("(%lf, %lf) -> (%lf, %lf)\n", ((GIMSEdge *)(*it))->p1->x, ((GIMSEdge *)(*it))->p1->y,
                                              ((GIMSEdge *)(*it))->p2->x, ((GIMSEdge *)(*it))->p2->y );
    }
}

GIMSGeometry *retrieveFeature ( OGRFeature *feature ) {

    GIMSGeometryList *geomlist = new GIMSGeometryList();
    OGRGeometry *geometry = feature->GetGeometryRef();
    
    if ( //geometry->getGeometryType() == wkbMultiPoint ||
         geometry->getGeometryType() == wkbPoint         ) {
        TODO(Implement support for point data loading)
        perror("called an unimplemented segment (retrieve a point OGRFeature)");
        exit(-1);

    } else if ( //geometry->getGeometryType() == wkbMultiLineString ||
                geometry->getGeometryType() == wkbLineString         ) {

        OGRLineString *lineString = ( (OGRLineString *) geometry);
        int N = lineString->getNumPoints();
        
        total += N-1;

        OGRPoint *tmp = new OGRPoint;
        lineString->getPoint (0, tmp);

        GIMSPoint *prev = NULL,
                  *curr = new GIMSPoint(tmp->getX(), tmp->getY());
        
        for (int i = 1; i < N; i++) {

            prev = curr;
            lineString->getPoint (i, tmp);
            curr = new GIMSPoint( tmp->getX(), tmp->getY() );
            
            if ( curr->x != prev->x || curr->y != prev->y ) {
                geomlist->list->push_back( new GIMSEdge(prev, curr) );
            }
        }
        
        delete tmp;
        return geomlist;
        
    } else if ( //geometry->getGeometryType() == wkbMultiPolygon ||
                geometry->getGeometryType() == wkbPolygon         ) {
        
        GIMSGeometryList *exteriorRing = new GIMSGeometryList();

        OGRLinearRing *extRing = ( (OGRPolygon *) geometry)->getExteriorRing();
        int N = extRing->getNumPoints();

        OGRPoint *tmp = new OGRPoint;
        extRing->getPoint (0, tmp);

        GIMSPoint *prev = NULL,
                  *curr = new GIMSPoint(tmp->getX(), tmp->getY());

        total += N+1;

        for (int i = 1; i < N; i++) {
            prev = curr;
            extRing->getPoint (i, tmp);
            curr = new GIMSPoint( tmp->getX(), tmp->getY() );
            
            if ( curr->x != prev->x || curr->y != prev->y ) {
                exteriorRing->list->push_back( new GIMSEdge(prev, curr) );
            }
        }
        

        int M = ( (OGRPolygon *) geometry)->getNumInteriorRings();
        GIMSGeometryList *interiorRings = new GIMSGeometryList();

        for( int k=0; k<M; k++ ){
            GIMSGeometryList *gimsIntRing = new GIMSGeometryList();
            
            OGRLinearRing *intRing = ( (OGRPolygon *) geometry)->getInteriorRing(k);
            N = intRing->getNumPoints();

            intRing->getPoint (0, tmp);

            GIMSPoint *prev = NULL,
                      *curr = new GIMSPoint(tmp->getX(), tmp->getY());

            for (int i = 1; i < N; i++) {
                prev = curr;
                intRing->getPoint (i, tmp);
                curr = new GIMSPoint( tmp->getX(), tmp->getY() );
                
                if ( curr->x != prev->x || curr->y != prev->y ) {
                    gimsIntRing->list->push_back( new GIMSEdge(prev, curr) );
                }
            }
            interiorRings->list->push_back(gimsIntRing);
        }
        delete tmp;

        GIMSPolygon *polygon = new GIMSPolygon(exteriorRing, interiorRings);
        geomlist->list->push_back( polygon );
        return geomlist;

    } else {
        perror ("unsupported type of geometry detected.");
        exit (-1);
    }

}
