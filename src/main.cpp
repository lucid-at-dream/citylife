//============================================================================
// Name        : PG2OGR.cpp
// Author      : Carlos Cacador
// Version     :
// Copyright   :
// Description : Hello World in C, Ansi-style
//============================================================================
/*
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include "Helper/PGConnection.h"
#include "Geometry/Connect2Postgis.h"

int main (void) {


    Geometry::Connect2Postgis getGeom;

    int size;


    //int count = getGeom.TestConnection();

    //std::cout << count << std::endl;


    //size = getGeom.CountIntersect();
    //size = getGeom.CountOverlaps();

    //std::cout << size << std::endl;

    getGeom.DumpPolygons();

    //OGRLineString** lineArray = getGeom.GeometryArray(size);
    //OGRLineString** lineArray = getGeom.GeometryArrayByDriver(size);

//      for (int i = 0; i < 100; i++)
//      {
//              std::cout << (*(lineArray[i])).exportToJson() << std::endl;
//      }

    //delete lineArray;

    return EXIT_SUCCESS;
}
*/

#include "PMQuadTree.hpp"
#include <cstdio>
#include <list>
#define INTERSECTION_COLOR "#D49"

/*
INPUT FORMAT:
upper limit (maxy), left limit (minx), right limit (maxx)
number of points
points (read in has a linestring)
*/

char colors [6][5] = {"#F00", "#0F0", "#00F", "#FF0", "#F0F", "#0FF"};

int main() {

    int M, N;
    double ls, ll, lr;
    
    scanf ("%lf %lf %lf", &ls, &ll, &lr);
    
    point *leftupper = createPoint (ll, ls);
    node *root = createLeafNode ( createSquare (leftupper, lr - ll) );
    std::list<edgelistNode *> *query;
    
    scanf ("%d", &M);
    
    for (int k = 0; k < M; k++) {
    
        std::list<edgelistNode *> *edges = new std::list<edgelistNode *>;
        scanf ("%d", &N);
        point *prev = NULL;
        point *curr = NULL;
        point *first = NULL;
        double x, y;
        
        for (int i = 0; i < N; i++) {
            scanf ("%lf %lf", &x, &y);
            curr = createPoint (x, y);
            
            if ( prev != NULL ) {
                edgelistNode *n = (edgelistNode *) calloc (1, sizeof (edgelistNode) );
                edgelistData *d = (edgelistData *) calloc (1, sizeof (edgelistData) );
                n->data = d;
                n->color = (char *) calloc (5, sizeof (char) );
                memcpy ( n->color, colors[k % 6], 5);
                n->datatype = EDGE;
                d->e = createEdge (prev, curr);
                
                edges->push_back (n);
                
            } else {
                first = curr;
            }
            
            prev = curr;
        }
        
        edgelistNode *n = (edgelistNode *) calloc (1, sizeof (edgelistNode) );
        edgelistData *d = (edgelistData *) calloc (1, sizeof (edgelistData) );
        n->data = d;
        n->color = (char *) calloc (5, sizeof (char) );
        memcpy ( n->color, colors[k % 6], 5);
        n->datatype = EDGE;
        d->e = createEdge (prev, first);
        edges->push_back (n);
        
        printf ("%d\n", k);
        
        if ( k > 0 ) {
            insert (edges, root);
            delete edges;
            
        } else {
            query = edges;
        }
    }
    
    //dumpTreeJSON (root);
    
    //intersects(root, query);
    
    return 0;
}

