#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "Geometry.hpp"
#include "DBConnection.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"

using namespace PMQUADTREE;
using namespace GIMS_GEOMETRY;

int total = 0;
unsigned long int incId = 1;

int ifilter_ContainedPoints(Node* n, GIMS_Geometry *polygon, GIMS_Geometry *g){
    if( g->type == POINT && n->polygonContainsPoint((GIMS_Polygon *)polygon, (GIMS_Point *)g))
        return 1;
    else
        return 0;
}

int cfilter_ContainedPoints(GIMS_Geometry *g){
    if(g->type == POINT)
        return 1;
    return 0;
}

void demo1();
void demo2();

int main (int argc, char *argv[]) {

    if( argc > 1 && strcmp(argv[1], "demo1") == 0 ){
        cout << "==== STARTING DEMO 1 ====\n";
        demo1();
        cout << "==== FINISHED DEMO 1 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo2") == 0 ){
        cout << "==== STARTING DEMO 2 ====\n";
        demo2();
        cout << "==== FINISHED DEMO 2 ====\n";
    }
    
    return 0;
}


void demo2(){
    PGConnection conn = PGConnection();
    conn.connect();

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    clock_t start, stop;

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *polygons = conn.getGeometry("from planet_osm_polygon where way_area > 500000000 LIMIT 1");
    list<GIMS_Geometry *> *points   = conn.getGeometry("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree->insert(polygons);
    tree->insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 polygon" << endl;

    start = clock();
    list<GIMS_Geometry *> *results = tree->getRelated(polygons->front(), ifilter_ContainedPoints, cfilter_ContainedPoints);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the contained points" << endl;

    cout << results->size() << " points are inside the polygon" << endl;

    for(list<GIMS_Geometry *>::iterator it=results->begin(); it != results->end(); it++){
        (*it)->type = POINT;
        tree->renderRed(*it);
    }
    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo2.svg", 400, 400);
    delete renderer;

    delete tree;
    for(list<GIMS_Geometry *>::iterator it = polygons->begin(); it!=polygons->end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete polygons;
    delete points;
}


void demo1(){
    PGConnection conn = PGConnection();
    conn.connect();

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    clock_t start, stop;

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *polygons = conn.getGeometry("from planet_osm_polygon where way_area > 500000000 LIMIT 1");
    list<GIMS_Geometry *> *points   = conn.getGeometry("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree->insert(polygons);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 polygon" << endl;

    start = clock();
    int ptcount = 0; 
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++){
        if( tree->contains(polygons->front(), *it) ){
            tree->renderRed(*it);
            ptcount++;
        }
    }
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to label all the points" << endl;

    cout << ptcount << " points are inside the polygon" << endl;

    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo1.svg", 400, 400);
    delete renderer;

    delete tree;
    for(list<GIMS_Geometry *>::iterator it = polygons->begin(); it!=polygons->end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete polygons;
    delete points;
}

