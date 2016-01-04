#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "Geometry.hpp"
#include "DBConnection.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"
#include "conf.hpp"

using namespace PMQUADTREE;
using namespace GIMS_GEOMETRY;

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

int lsFilter(GIMS_Geometry *geom){
    if(geom->type == LINESTRING || geom->type == MULTILINESTRING)
        return 1;
    return 0;
}

int main(int argc, char **argv){

    char line[4096];
    int status;
    
    status = scanf("%[^\n]", line); getchar();
    if( status == EOF ) return -1;
    GIMS_Polygon *p1 = (GIMS_Polygon *)fromWkt(line);

    status = scanf("%[^\n]", line); getchar();
    if( status == EOF ) return -1;
    GIMS_Polygon *p2 = (GIMS_Polygon *)fromWkt(line);

    status = scanf("%[^\n]", line); getchar();
    if( status == EOF ) return -1;
    GIMS_Point *lowerLeft = (GIMS_Point *)fromWkt(line);

    status = scanf("%[^\n]", line); getchar();
    if( status == EOF ) return -1;
    GIMS_Point *upperRight = (GIMS_Point *)fromWkt(line);

    GIMS_BoundingBox *domain = new GIMS_BoundingBox(lowerLeft, upperRight);

    GIMS_Polygon *clipped1  = (GIMS_Polygon *)(p1->clipToBox( domain ));
    GIMS_Polygon *clipped2 = (GIMS_Polygon *)(p2->clipToBox( domain ));

    buildPlanarGraph(clipped1, clipped2, domain);

    return 0;
}

/*
int main(int argc, char **argv){
    
    //1. read and load configurations
    if( conf::readConfigurationFiles(argc, argv) != 0 )
        return -1;
    conf::printCurrentConfiguration();

    //2. load data from data sources
    clock_t start, stop;

    start = clock();
    PGConnection conn = PGConnection();
    conn.connect();
    stop = clock();
    cout << "DBCONN: " << (stop - start)/(double)CLOCKS_PER_SEC << " seconds" << endl;

    //retrieve layers extent
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    //create an empty pmqtree bounded by the computed extent
    PMQuadTree tree = PMQuadTree( extent );

    char fromClause[512];
    for( list<char *>::iterator it = configuration.db_layers.begin(); it != configuration.db_layers.end(); it++ ){
        start = clock();
        sprintf(fromClause, "FROM %s", *it);
        list<GIMS_Geometry *> objects = conn.getGeometryAsList(fromClause);
        stop = clock();

        cout << "DBFETCH: " << *it << ": " << (stop - start)/(double)CLOCKS_PER_SEC << " seconds" << endl;

        start = clock();
        tree.insert(objects);
        stop = clock();
        cout << "PMQTINS: " << *it << ": " << objects.size() << " elements: " << (stop - start)/(double)CLOCKS_PER_SEC << " seconds" << endl;

    }
    conn.disconnect();

    //3. start server for querying

    //4. [Optional]render
    if( 0 ){
        renderer = DebRenderer(&tree);
        renderer.setScale(400.0/extent->xlength(), -400.0/extent->ylength());
        renderer.setTranslation( -extent->minx(), -extent->maxy() );
        renderer.mainloop(argc, argv);
    }

    //5. clean exit
}
*/