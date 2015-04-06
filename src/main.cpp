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

int pointFilter(GIMS_Geometry *geom){
    if(geom->type == POINT)
        return 1;
    return 0;
}

void demo1();
void demo2();
void demo3();
void demo4();
void demo5();

int main (int argc, char *argv[]) {

    if( argc > 1 && strcmp(argv[1], "demo1") == 0 ){
        cout << "==== STARTING DEMO 1 ====\n";
        demo1();
        cout << "==== FINISHED DEMO 1 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo2") == 0 ){
        cout << "==== STARTING DEMO 2 ====\n";
        demo2();
        cout << "==== FINISHED DEMO 2 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo3") == 0 ){
        cout << "==== STARTING DEMO 3 ====\n";
        demo3();
        cout << "==== FINISHED DEMO 3 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo4") == 0 ){
        cout << "==== STARTING DEMO 4 ====\n";
        demo4();
        cout << "==== FINISHED DEMO 4 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo5") == 0 ){
        cout << "==== STARTING DEMO 5 ====\n";
        demo5();
        cout << "==== FINISHED DEMO 5 ====\n";
    }else{

        PGConnection conn = PGConnection();
        conn.connect();

        GIMS_BoundingBox *extent = conn.getOverallExtent();
        PMQuadTree *tree = new PMQuadTree( extent );

        list<GIMS_Geometry *> *polygons = conn.getGeometryAsList("from planet_osm_polygon");
        cout << "polygons loaded" << endl;
        list<GIMS_Geometry *> *lines    = conn.getGeometryAsList("from planet_osm_line");
        cout << "lines loaded" << endl;
        list<GIMS_Geometry *> *roads    = conn.getGeometryAsList("from planet_osm_roads");
        cout << "roads loaded" << endl;
        list<GIMS_Geometry *> *points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
        cout << "points loaded" << endl;

        clock_t start, stop;

        start = clock();
        tree->insert(polygons);
        cout << "polygons inserted" << endl;
        cout << "tree maximum depth: " << tree->getMaxDepth() << endl;
        cout << "total number of nodes: " << tree->getNumNodes() << endl;


        tree->insert(points);
        cout << "points inserted" << endl;
        cout << "tree maximum depth: " << tree->getMaxDepth() << endl;
        cout << "total number of nodes: " << tree->getNumNodes() << endl;


        tree->insert(lines);
        cout << "lines inserted" << endl;
        cout << "tree maximum depth: " << tree->getMaxDepth() << endl;
        cout << "total number of nodes: " << tree->getNumNodes() << endl;


        tree->insert(roads);
        cout << "roads inserted" << endl;
        cout << "tree maximum depth: " << tree->getMaxDepth() << endl;
        cout << "total number of nodes: " << tree->getNumNodes() << endl;
        stop = clock();

        cout << "took ";
        cout << (stop - start)/(double)CLOCKS_PER_SEC;
        cout << " seconds to build the whole index" << endl;

        /*
        tree->query = polygons->front();
        renderer = new DebRenderer();
        renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
        renderer->setTranslation( -extent->minx(), -extent->maxy() );
        renderer->renderCallback = tree;
        renderer->mainloop(argc, argv);
        */

        delete renderer;
        delete tree;
        for(list<GIMS_Geometry *>::iterator it = roads->begin(); it!=roads->end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = lines->begin(); it!=lines->end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = polygons->begin(); it!=polygons->end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
            (*it)->deepDelete();
        delete polygons;
        delete points;
        delete roads;
        delete lines;
    }

    return 0;
}

void demo5(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> *points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
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

    int count;
    for(int topcount=0; topcount<25001; topcount+=250){
        
        count = 0;

        list<GIMS_Geometry *>::iterator it;
        start = clock();
        for( it = polygons->begin(); it != polygons->end(); it++){        
            DE9IM *results = tree->topologicalSearch(*it, pointFilter);
            list<long> covered_pts = results->covers();

            //for(list<long>::iterator j = covered_pts.begin(); j != covered_pts.end(); j++){
            //    cout << (*it)->id << " | " << *j << endl;
            //}

            delete results;

            count++;
            if( count >= topcount )
                break;
        }
        stop = clock();

        cout << "N: " << count << endl;
        cout << "took: " << (stop - start)/(double)CLOCKS_PER_SEC << " seconds" << endl;

        if( it == polygons->end() )
            break;
    }
}

int polygonFilter(GIMS_Geometry *geom){
    if(geom->type == POLYGON || geom->type == MULTIPOLYGON)
        return 1;
    return 0;
}

void demo4(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> *points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
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
    cout << " seconds to insert the " << polygons->size() << " polygons";
    cout << " and the " << points->size() << " points" << endl;

    int count;
    for(int counttop = 0; counttop < 160000; counttop += 250){
        
        count = 0;
        list<GIMS_Geometry *>::iterator it;
        start = clock();
        for(it = points->begin(); it != points->end(); it++){
            DE9IM *results = tree->topologicalSearch(*it, polygonFilter);
            list<long> covered_pts = results->coveredBy();

            //for(list<long>::iterator j = covered_pts.begin(); j != covered_pts.end(); j++){
            //    cout << (*it)->id << " | " << *j << endl;
            //}

            delete results;
            count++;

            if(count >= counttop)
                break;
        }
        stop = clock();

        cout << "N: " << count << endl;
        cout << "took: " << (stop - start)/(double)CLOCKS_PER_SEC << "seconds" << endl;

        if( it == points->end() )
            break;
    }

    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo4.svg", 400, 400);

    char *argv[] = {"gims", "demo4"};
    int argc = 2;
    renderer->mainloop(argc, argv);
    
    delete renderer;


    delete tree;
    for(list<GIMS_Geometry *>::iterator it = polygons->begin(); it!=polygons->end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete polygons;
    delete points;    
}

int lsFilter(GIMS_Geometry *geom){
    if(geom->type == LINESTRING || geom->type == MULTILINESTRING)
        return 1;
    return 0;
}

void demo3(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *lines = conn.getGeometryAsList("from planet_osm_line");
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree->insert(lines);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the " << lines->size() << " linestrings" << endl;

    start = clock();
    GIMS_LineString query;
    query.id = 2954258;
    DE9IM *results = tree->topologicalSearch(*(idIndex.find(&query)), lsFilter);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the related linestrings" << endl;

    list<long> intersected = results->intersects();
    cout << intersected.size() << " lines intersect the given line" << endl;

    for(list<long>::iterator it=intersected.begin(); it != intersected.end(); it++){
        query.id = *it;
        GIMS_Geometry *result = *(idIndex.find(&query));
        tree->renderRed(result);
        cout << *it << endl;
    }

    tree->query = NULL;
    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo3.svg", 400, 400);

    char *argv[] = {"gims", "demo3"};
    int argc = 2;
    renderer->mainloop(argc, argv);
    
    delete renderer;
    delete results;

    delete tree;
    for(list<GIMS_Geometry *>::iterator it = lines->begin(); it!=lines->end(); it++)
        (*it)->deepDelete();
    delete lines;
}

/*
void demo4(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    GIMS_BoundingBox *extent = conn.getOverallExtent();

    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *lines = conn.getGeometryAsList("from planet_osm_line where random() < 0.1");
    list<GIMS_Geometry *> *points= conn.getGeometryAsList("from planet_osm_point LIMIT 100000");
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree->insert(lines);
    tree->insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 linestring" << endl;

    start = clock();

    list<GIMS_Geometry *> *results;
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it != points->end(); it++){
        
        results = tree->activeSearch(points->top(), sfilter_PointCoveredByLines);
        if(results->size() > 0){
            cout << results->size() << " lines cover one of the points." << endl;
        
            tree->renderRed(*it);
            for(list<GIMS_Geometry *>::iterator res_it=results->begin(); res_it != results->end(); res_it++){
                tree->renderRed(*res_it);
            }
        }        
        delete results;
    }
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the covered points" << endl;

    tree->query = points->top();
    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo4.svg", 400, 400);
    
    char *argv[] = {"gims", "demo4"};
    int argc = 2;
    renderer->mainloop(argc, argv);
    
    delete renderer;

    delete tree;
    for(list<GIMS_Geometry *>::iterator it = lines->begin(); it!=lines->end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete lines;
    delete points;
}

void demo3(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    GIMS_BoundingBox *extent = conn.getOverallExtent();

    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    AVLTree<long, GIMS_Geometry *> *lines = conn.getGeometry("from planet_osm_line where osm_id=96679093");
    AVLTree<long, GIMS_Geometry *> *points   = conn.getGeometry("from planet_osm_point");
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree->insert(lines);
    tree->insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 linestring" << endl;

    start = clock();
    AVLTree<long, GIMS_Geometry *> *results = tree->activeSearch(lines->top(), sfilter_LineCoveredPoints);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the covered points" << endl;

    cout << results->size() << " points are covered by the linestring" << endl;

    for(AVLTree<long, GIMS_Geometry *>::iterator it=results->begin(); it != results->end(); it++){
        (*it)->type = POINT;
        tree->renderRed(*it);
    }
    tree->query = lines->top();
    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo3.svg", 400, 400);
    
    char *argv[] = {"gims", "demo3"};
    int argc = 2;
    renderer->mainloop(argc, argv);
    
    delete renderer;

    delete tree;
    for(AVLTree<long, GIMS_Geometry *>::iterator it = lines->begin(); it!=lines->end(); it++)
        (*it)->deepDelete();
    for(AVLTree<long, GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete lines;
    delete points;
}
*/

void demo2(){
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to connect to the database" << endl;

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> *polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> *points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
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

    GIMS_Polygon query; query.id = -1715038;
    GIMS_Geometry *found = *(idIndex.find(&query));

    start = clock();
    DE9IM *results = tree->topologicalSearch(found, pointFilter);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the contained points" << endl;

    list<long> covered_pts = results->covers();
    cout << covered_pts.size() << " points are inside the polygon" << endl;

    for(list<long>::iterator it=covered_pts.begin(); it != covered_pts.end(); it++){
        for(list<GIMS_Geometry *>::iterator it_j = points->begin(); it_j != points->end(); it_j++){
            if( (*it) == (*it_j)->id )
                tree->renderRed(*it_j);
        }
    }

    tree->query = polygons->front();
    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    renderer->renderSvg("demo2.svg", 400, 400);

    char *argv[] = {"gims", "demo2"};
    int argc = 2;
    renderer->mainloop(argc, argv);
    
    delete renderer;
    delete results;

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
    list<GIMS_Geometry *> *polygons = conn.getGeometryAsList("from planet_osm_polygon where osm_id=-1715038"); /*4532 points*/
    list<GIMS_Geometry *> *points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
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
        list<Node *> *l = (list<Node *> *)(tree->search(*it));
        Node *n = l->front();
        delete l;
        if(n->polygonContainsPoint((GIMS_Polygon *)(polygons->front()), (GIMS_Point *)(*it))){
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

    tree->query = polygons->front();
    char *argv[] = {"gims", "demo2"};
    int argc = 2;
    renderer->mainloop(argc, argv);

    delete renderer;

    delete tree;
    for(list<GIMS_Geometry *>::iterator it = polygons->begin(); it!=polygons->end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points->begin(); it!=points->end(); it++)
        (*it)->deepDelete();
    delete polygons;
    delete points;
}

