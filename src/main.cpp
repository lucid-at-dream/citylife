#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include "tunasolver.hpp"

using namespace std;

void pointBenchmark();
void linestringBenchmark();
void polygonBenchmark();

int main(int argc, char **argv){

    //1. load configuration
    if( loadConfiguration(argc, argv) )
        return -1;

    //2. connect to database
    connectToDatabase();
    GIMS_BoundingBox *extent = dbconnection.getOverallExtent();

    //3. create an index
    spatialIndex *index = createSpatialIndex(extent);

    //4. load data from db
    list<GIMS_Geometry *> planet_osm_point   = loadPostgisData("FROM planet_osm_point");
    list<GIMS_Geometry *> planet_osm_line    = loadPostgisData("FROM planet_osm_line");
    list<GIMS_Geometry *> planet_osm_polygon = loadPostgisData("FROM planet_osm_polygon");

    //5. insert the data in the index
    index->insert(planet_osm_point);
    index->insert(planet_osm_line);
    index->insert(planet_osm_polygon);

    //6. perform benchmarks
    pointBenchmark();
    linestringBenchmark();
    polygonBenchmark();
    
    //renderIndex(index, extent);
    delete index;
    shutdownTunaSolver();
}

void pointBenchmark(){

}

void linestringBenchmark(){

}

void polygonBenchmark(){
    
}

