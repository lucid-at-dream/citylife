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
    
    OGRLayer *layer = getGeom.GetLayerByName ("planet_osm_line");
    
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
    
    while ( (feature = layer->GetNextFeature() ) != NULL) {
    
        if ( strcmp (feature->GetFieldAsString (feature->GetFieldIndex ("highway") ), "cycleway") != 0 ) {
            delete feature;
            continue;
        }
        
        tree->insert ( retrieveFeature (feature) );
        delete feature;
    }

    printf("inserted %d edges.\n", total);
    
    renderer = new DebRenderer();
    renderer->renderCallback = tree;
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
        
    } else if ( //geometry->getGeometryType() == wkbMultiPolygon ||
                geometry->getGeometryType() == wkbPolygon         ) {
                
        // OGRLinearRing *extRing = ( (OGRPolygon *) geometry)->getExteriorRing();
        // int N = extRing->getNumPoints();
        // std::list<edgelistNode *> *t_extRing = new std::list<edgelistNode *>();
        
        // for ( int i = 0; i < N; i++ ) {
        //     edgelistNode *n = (edgelistNode *) calloc (1, sizeof (edgelistNode) );
        //     n->datatype = EXT_RING;
        //     n->data = (edgelistData *) calloc (1, sizeof (edgelistData) );
        //     n->data->ls = createLineSeg ( extRing, i );
        //     n->feature = feature;
        //     t_extRing->push_back ( n );
        // }
        
        // insert (t_extRing, R);
        // delete t_extRing;
        
    } else {
        perror ("unsupported type of geometry detected.");
        exit (-1);
    }
    
    return geomlist;
}
