//============================================================================
// Name        : PG2OGR.cpp
// Author      : Carlos Cacador
// Version     :
// Copyright   :
// Description : Hello World in C, Ansi-style
//============================================================================

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include "PGConnection.h"
#include "Connect2Postgis.h"
#include "PMQuadTree.hpp"
#define INTERSECTION_COLOR "#D49"

char colors [6][5] = {"#F00", "#0F0", "#00F", "#FF0", "#F0F", "#0FF"};

std::list<edgelistNode *> *retrieveFeature ( OGRFeature *feature );
void dumpList (std::list<edgelistNode *> *list);

int main (void) {

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
           
    point *leftupper = createPoint (envelope->MinX, envelope->MaxY);
    node *root = createLeafNode ( createSquare (leftupper, len) );
    delete envelope;
    
    OGRFeature *feature;
    std::list<edgelistNode *> *list;
    
    while ( (feature = layer->GetNextFeature() ) != NULL) {
    
        if ( strcmp (feature->GetFieldAsString (feature->GetFieldIndex ("highway") ),
                     "cycleway") != 0 ) {
            delete feature;
            continue;
        }
        
        list = retrieveFeature (feature);
        insert (list, root);
        delete feature;
        delete list;
    }
    
    delete layer;
    
    dumpTreeJSON (root);
    
    return EXIT_SUCCESS;
}

void dumpList (std::list<edgelistNode *> *list) {
    std::list<edgelistNode *>::iterator it = list->begin();
    
    for (; it != list->end(); it++) {
        printf ("(%lf, %lf) -> (%lf, %lf)\n", (*it)->data->e->p1->x,
                (*it)->data->e->p1->y,
                (*it)->data->e->p2->x, (*it)->data->e->p2->y);
    }
    
}

std::list<edgelistNode *> *retrieveFeature ( OGRFeature *feature ) {

    std::list<edgelistNode *> *list = new std::list<edgelistNode *>();
    OGRGeometry *geometry = feature->GetGeometryRef();
    int color = rand() % 6;
    
    if ( geometry->getGeometryType() == wkbPoint ||
            geometry->getGeometryType() == wkbMultiPoint ) {
            
    } else if ( geometry->getGeometryType() == wkbLineString ||
                geometry->getGeometryType() == wkbMultiLineString ) {
                
        OGRLineString *lineString = ( (OGRLineString *) geometry);
        int N = lineString->getNumPoints();
        
        point *prev = NULL,
               *curr = (point *) calloc (1, sizeof (point) );
        OGRPoint *tmp = new OGRPoint;
        lineString->getPoint (0, tmp);
        curr->x = tmp->getX(); curr->y = tmp->getY();
        
        for (int i = 1; i < N; i++) {
            edgelistNode *n = (edgelistNode *) calloc (1, sizeof (edgelistNode) );
            edgelistData *d = (edgelistData *) calloc (1, sizeof (edgelistData) );
            n->data = d;
            n->color = (char *) calloc (5, sizeof (char) );
            memcpy ( n->color, colors[color], 5);
            n->datatype = EDGE;
            
            prev = curr;
            lineString->getPoint (i, tmp);
            curr = (point *) calloc (1, sizeof (point) );
            curr->x = tmp->getX(); curr->y = tmp->getY();
            
            if ( curr->x != prev->x || curr->y != prev->y ) {
                d->e = createEdge (prev, curr);
                list->push_back (n);
            }
        }
        
        delete tmp;
        
    } else if ( geometry->getGeometryType() == wkbPolygon ||
                geometry->getGeometryType() == wkbMultiPolygon ) {
                
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
    
    return list;
}
