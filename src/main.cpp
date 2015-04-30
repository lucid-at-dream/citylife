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

int polygonFilter(GIMS_Geometry *geom){
    if(geom->type == POLYGON || geom->type == MULTIPOLYGON)
        return 1;
    return 0;
}

void demo1();
void demo2();
void demo3();
void demo4();
void demo5();
void demo6();
void demo7();
void demo8();

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
    }else if( argc > 1 && strcmp(argv[1], "demo6") == 0 ){
        cout << "==== STARTING DEMO 6 ====\n";
        demo6();
        cout << "==== FINISHED DEMO 6 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo7") == 0 ){
        cout << "==== STARTING DEMO 7 ====\n";
        demo7();
        cout << "==== FINISHED DEMO 7 ====\n";
    }else if( argc > 1 && strcmp(argv[1], "demo8") == 0 ){
        cout << "==== STARTING DEMO 8 ====\n";
        demo8();
        cout << "==== FINISHED DEMO 8 ====\n";
    }else{

        PGConnection conn = PGConnection();
        conn.connect();

        GIMS_BoundingBox *extent = conn.getOverallExtent();
        PMQuadTree tree = PMQuadTree( extent );

        list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon");
        cout << "polygons loaded" << endl;
        list<GIMS_Geometry *> lines    = conn.getGeometryAsList("from planet_osm_line");
        cout << "lines loaded" << endl;
        list<GIMS_Geometry *> roads    = conn.getGeometryAsList("from planet_osm_roads");
        cout << "roads loaded" << endl;
        list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
        cout << "points loaded" << endl;

        clock_t start, stop;

        start = clock();
        tree.insert(polygons);
        cout << "polygons inserted" << endl;
        cout << "tree maximum depth: " << tree.getMaxDepth() << endl;
        cout << "total number of nodes: " << tree.getNumNodes() << endl;


        tree.insert(points);
        cout << "points inserted" << endl;
        cout << "tree maximum depth: " << tree.getMaxDepth() << endl;
        cout << "total number of nodes: " << tree.getNumNodes() << endl;


        tree.insert(lines);
        cout << "lines inserted" << endl;
        cout << "tree maximum depth: " << tree.getMaxDepth() << endl;
        cout << "total number of nodes: " << tree.getNumNodes() << endl;


        tree.insert(roads);
        cout << "roads inserted" << endl;
        cout << "tree maximum depth: " << tree.getMaxDepth() << endl;
        cout << "total number of nodes: " << tree.getNumNodes() << endl;
        stop = clock();

        cout << "took ";
        cout << (stop - start)/(double)CLOCKS_PER_SEC;
        cout << " seconds to build the whole index" << endl;

        /*
        tree.query = polygons.front();
        renderer = DebRenderer(&tree);
        renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
        renderer.setTranslation( -extent->minx(), -extent->maxy() );
        renderer.mainloop(argc, argv);
        */

        for(list<GIMS_Geometry *>::iterator it = roads.begin(); it!=roads.end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = lines.begin(); it!=lines.end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it!=polygons.end(); it++)
            (*it)->deepDelete();
        for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++)
            (*it)->deepDelete();
    }

    return 0;
}

void demo8(){

    printf("connecting\n");
    PGConnection conn = PGConnection();
    conn.connect();
    
    GIMS_BoundingBox *extent = conn.getOverallExtent();
    PMQuadTree tree = PMQuadTree( extent );
    
    printf("fetching stuff\n");
    list<GIMS_Geometry *> roads_1 = conn.getGeometryAsList("from planet_osm_roads WHERE highway='primary'");
    tree.insert(roads_1);
    list<GIMS_Geometry *> roads_2 = conn.getGeometryAsList("from planet_osm_roads WHERE highway='secondary'");
    tree.insert(roads_2);
    list<GIMS_Geometry *> roads_3 = conn.getGeometryAsList("from planet_osm_roads WHERE highway='cycleway'");
    tree.insert(roads_3);
    list<GIMS_Geometry *> roads_4 = conn.getGeometryAsList(
        "from planet_osm_roads WHERE"
        " highway!='primary' AND"
        " highway!='secondary' AND"
        " highway!='cycleway'"
    );
    tree.insert(roads_4);

    tree.dumpLevita();

    for(list<GIMS_Geometry *>::iterator it = roads_1.begin(); it != roads_1.end(); it++){
        GIMS_LineString *l = (GIMS_LineString *)(*it);
        for(int i=0; i<l->size; i++ ){
            GIMS_Point *p = l->list[i];
            printf("LINE_P: %llu %lf %lf p\n", pointcount++, p->x, p->y);
            if( i>0 )
                printf("LINE_L: %llu %llu p\n", pointcount-2, pointcount-1);
        }
    }

    for(list<GIMS_Geometry *>::iterator it = roads_2.begin(); it != roads_2.end(); it++){
        GIMS_LineString *l = (GIMS_LineString *)(*it);
        for(int i=0; i<l->size; i++ ){
            GIMS_Point *p = l->list[i];
            printf("LINE_P: %llu %lf %lf s\n", pointcount++, p->x, p->y);
            if( i>0 )
                printf("LINE_L: %llu %llu s\n", pointcount-2, pointcount-1);
        }
    }

    for(list<GIMS_Geometry *>::iterator it = roads_3.begin(); it != roads_3.end(); it++){
        GIMS_LineString *l = (GIMS_LineString *)(*it);
        for(int i=0; i<l->size; i++ ){
            GIMS_Point *p = l->list[i];
            printf("LINE_P: %llu %lf %lf c\n", pointcount++, p->x, p->y);
            if( i>0 )
                printf("LINE_L: %llu %llu c\n", pointcount-2, pointcount-1);
        }
    }

    for(list<GIMS_Geometry *>::iterator it = roads_4.begin(); it != roads_4.end(); it++){
        GIMS_LineString *l = (GIMS_LineString *)(*it);
        for(int i=0; i<l->size; i++ ){
            GIMS_Point *p = l->list[i];
            printf("LINE_P: %llu %lf %lf o\n", pointcount++, p->x, p->y);
            if( i>0 )
                printf("LINE_L: %llu %llu o\n", pointcount-2, pointcount-1);
        }
    }
}

void demo7(){
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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    tree.insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the " << polygons.size() << " polygons";
    cout << " and the " << points.size() << " points" << endl;

    start = clock();
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it != points.end(); it++){
        DE9IM *results = tree.topologicalSearch(*it, polygonFilter);
        list<long> covered_pts = results->coveredBy();

        for(list<long>::iterator j = covered_pts.begin(); j != covered_pts.end(); j++){
            GIMS_Point p; p.id = *j;
            cout << (*it)->osm_id << " | " << (*(idIndex.find(&p)))->osm_id << endl;
        }

        delete results;
    }
    stop = clock();

    cout << "took: " << (stop - start)/(double)CLOCKS_PER_SEC << "seconds" << endl;

    for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it!=polygons.end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++)
        (*it)->deepDelete();
}

void demo6(){
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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    tree.insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the data" << endl;

    start = clock();
    for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it != polygons.end(); it++){

        DE9IM *results = tree.topologicalSearch(*it, pointFilter);
        list<long> covered_pts = results->covers();

        for(list<long>::iterator j = covered_pts.begin(); j != covered_pts.end(); j++){
            GIMS_Point p; p.id = *j;
            GIMS_Point *l_pt = (GIMS_Point *)(*(idIndex.find(&p)));
            cout << (*it)->osm_id << " | " << l_pt->osm_id << endl;
        }

        delete results;
    }
    stop = clock();

    cout << "took: " << (stop - start)/(double)CLOCKS_PER_SEC << " seconds" << endl;

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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    tree.insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 polygon" << endl;

    list<GIMS_Geometry *>::iterator it;
    start = clock();
    for( it = polygons.begin(); it != polygons.end(); it++){        
        DE9IM *results = tree.topologicalSearch(*it, pointFilter);
        list<long> covered_pts = results->covers();

        for(list<long>::iterator j = covered_pts.begin(); j != covered_pts.end(); j++){
            GIMS_Point p; p.id = *j;
            GIMS_Point *l_pt = (GIMS_Point *)(*(idIndex.find(&p)));
            cout << (*it)->osm_id << " | " << l_pt->osm_id << endl;
        }

        delete results;
    }
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to compute the points" << endl;

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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    tree.insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the " << polygons.size() << " polygons";
    cout << " and the " << points.size() << " points" << endl;

    int count;
    for(int counttop = 0; counttop < 150001; counttop += 1500){
        
        count = 0;
        list<GIMS_Geometry *>::iterator it;
        start = clock();
        for(it = points.begin(); it != points.end(); it++){
            DE9IM *results = tree.topologicalSearch(*it, polygonFilter);
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

        if( it == points.end() )
            break;
    }

    renderer = DebRenderer(&tree);
    renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer.setTranslation( -extent->minx(), -extent->maxy() );
    renderer.renderSvg("demo4.svg", 400, 400);

    char *argv[] = {"gims", "demo4"};
    int argc = 2;
    renderer.mainloop(argc, argv);
    


    for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it!=polygons.end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++)
        (*it)->deepDelete();
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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> lines = conn.getGeometryAsList("from planet_osm_line"
        /*" WHERE "
        "osm_id = -3143502 OR "
        "osm_id = 139591775 OR "
        "osm_id = 149905193 OR "
        "osm_id = 149905194 OR "
        "osm_id = 149905194 OR "
        "osm_id = 152532787 OR "
        "osm_id = 233492030 OR "
        "osm_id = 233493874 OR "
        "osm_id = 233495893 OR "
        "osm_id = 233761343"*/);
    conn.disconnect();
    stop = clock();


    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(lines);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the " << lines.size() << " linestrings" << endl;

    int total = 0;

    start = clock();
    for(list<GIMS_Geometry *>::iterator it = lines.begin(); it != lines.end(); it++){
        
        DE9IM *results = tree.topologicalSearch(*it, lsFilter);
        list<long> intersected = results->intersects();
        
        for( list<long>::iterator k = intersected.begin(); k != intersected.end(); k++ ){
            GIMS_LineString query; query.id = *k;
            cout <<  (*it)->osm_id << " | " << (*(idIndex.find(&query)))->osm_id << endl;
            total++;
        }

        delete results;
    }
    stop = clock();
    
    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the " << total << " related linestrings" << endl;
    
    /*renderer = DebRenderer(&tree);
    renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer.setTranslation( -extent->minx(), -extent->maxy() );
    renderer.renderSvg("demo2.svg", 400, 400);

    char *argv[] = {"gims", "demo2"};
    int argc = 2;
    renderer.mainloop(argc, argv);*/
    
    for(list<GIMS_Geometry *>::iterator it = lines.begin(); it!=lines.end(); it++)
        (*it)->deepDelete();
}

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
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    tree.insert(points);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 polygon" << endl;

    GIMS_Polygon query; query.id = -1715038;
    GIMS_Geometry *found = *(idIndex.find(&query));

    start = clock();
    DE9IM *results = tree.topologicalSearch(found, pointFilter);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to find all the contained points" << endl;

    list<long> covered_pts = results->covers();
    cout << covered_pts.size() << " points are inside the polygon" << endl;

    for(list<long>::iterator it=covered_pts.begin(); it != covered_pts.end(); it++){
        for(list<GIMS_Geometry *>::iterator it_j = points.begin(); it_j != points.end(); it_j++){
            if( (*it) == (*it_j)->id )
                tree.renderRed(*it_j);
        }
    }

    tree.query = polygons.front();
    renderer = DebRenderer(&tree);
    renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer.setTranslation( -extent->minx(), -extent->maxy() );
    renderer.renderSvg("demo2.svg", 400, 400);

    char *argv[] = {"gims", "demo2"};
    int argc = 2;
    renderer.mainloop(argc, argv);
    
    delete results;

    for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it!=polygons.end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++)
        (*it)->deepDelete();
}


void demo1(){
    PGConnection conn = PGConnection();
    conn.connect();

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    clock_t start, stop;

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree tree = PMQuadTree( extent );

    start = clock();
    list<GIMS_Geometry *> polygons = conn.getGeometryAsList("from planet_osm_polygon where osm_id=-1715038"); /*4532 points*/
    list<GIMS_Geometry *> points   = conn.getGeometryAsList("from planet_osm_point"); /*152675 points*/
    conn.disconnect();
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to fetch all the data" << endl;

    start = clock();
    tree.insert(polygons);
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to insert the 1 polygon" << endl;

    start = clock();
    int ptcount = 0; 
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++){
        list<Node *> *l = (list<Node *> *)(tree.search(*it));
        Node *n = l->front();
        delete l;
        if(n->polygonContainsPoint((GIMS_Polygon *)(polygons.front()), (GIMS_Point *)(*it))){
            tree.renderRed(*it);
            ptcount++;
        }
    }
    stop = clock();

    cout << "took ";
    cout << (stop - start)/(double)CLOCKS_PER_SEC;
    cout << " seconds to label all the points" << endl;

    cout << ptcount << " points are inside the polygon" << endl;

    renderer = DebRenderer(&tree);
    renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer.setTranslation( -extent->minx(), -extent->maxy() );
    renderer.renderSvg("demo1.svg", 400, 400);

    tree.query = polygons.front();
    char *argv[] = {"gims", "demo2"};
    int argc = 2;
    renderer.mainloop(argc, argv);

    for(list<GIMS_Geometry *>::iterator it = polygons.begin(); it!=polygons.end(); it++)
        (*it)->deepDelete();
    for(list<GIMS_Geometry *>::iterator it = points.begin(); it!=points.end(); it++)
        (*it)->deepDelete();
}

