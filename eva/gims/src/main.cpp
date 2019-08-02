#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <list>
#include "tunasolver.hpp"

using namespace std;

void benchmark(spatialIndex *, list<GIMS_Geometry *> &, list<GIMS_Geometry *> &, list<GIMS_Geometry *> &);
void validate(spatialIndex *, list<GIMS_Geometry *> &, list<GIMS_Geometry *> &, list<GIMS_Geometry *> &);

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
    cout << "loading planet_osm_point" << endl;
    list<GIMS_Geometry *> planet_osm_point   = loadPostgisData("FROM planet_osm_point ORDER BY osm_id DESC LIMIT 500");
    cout << "loading planet_osm_line" << endl;
    list<GIMS_Geometry *> planet_osm_line    = loadPostgisData("FROM planet_osm_line ORDER BY osm_id ASC LIMIT 25");
    cout << "loading planet_osm_polygon" << endl;
    list<GIMS_Geometry *> planet_osm_polygon = loadPostgisData("FROM planet_osm_polygon ORDER BY osm_id ASC LIMIT 25");
    cout << "finished loading objects" << endl;



    //load id list geometric objects
    FILE *f; long long int id; char buffer[254]; int queryidslimit = 10000;

    f = fopen("sqldata/idlists/pt","r");
    list<GIMS_Geometry *> query_points;
    while(fscanf(f, "%lld", &id) != EOF){
        
        for( list<GIMS_Geometry *>::iterator it = planet_osm_point.begin(); it != planet_osm_point.end(); it++ ){
            if( (*it)->osm_id == id ){
                query_points.push_back(*it);
                break;
            }
        }
        if( query_points.size() > queryidslimit )
            break;
    }
    fclose(f);
    printf("num query points: %d\n", query_points.size());

    f = fopen("sqldata/idlists/ls","r");
    list<GIMS_Geometry *> query_lines;
    while(fscanf(f, "%lld", &id) != EOF){
        for( list<GIMS_Geometry *>::iterator it = planet_osm_line.begin(); it != planet_osm_line.end(); it++ ){
            if( (*it)->osm_id == id ){
                query_lines.push_back(*it);
                break;
            }
        }
        if( query_lines.size() > queryidslimit )
            break;
    }
    fclose(f);
    printf("num query lines: %d\n", query_lines.size());

    f = fopen("sqldata/idlists/pol","r");
    list<GIMS_Geometry *> query_polygons;
    while(fscanf(f, "%lld", &id) != EOF){
        for( list<GIMS_Geometry *>::iterator it = planet_osm_polygon.begin(); it != planet_osm_polygon.end(); it++ ){
            if( (*it)->osm_id == id ){
                query_polygons.push_back(*it);
                break;
            }
        }
        if( query_polygons.size() > queryidslimit )
            break;
    }
    fclose(f);
    printf("num query polygons: %d\n", query_polygons.size());

    printf("Finished inserting query objects\n");



    for( list<GIMS_Geometry *>::iterator it = planet_osm_polygon.begin(); it != planet_osm_polygon.end(); it++ ){
        if( ((GIMS_Polygon *)(*it))->internalRings != NULL )
            printf("ignore >>> %lld\n", (*it)->osm_id);
    }

    //5. insert the data in the index
    cout << "indexing planet_osm_point" << endl;
    index->insert(planet_osm_point);
    cout << "indexing planet_osm_line" << endl;
    index->insert(planet_osm_line);
    cout << "indexing planet_osm_polygon" << endl;
    index->insert(planet_osm_polygon);
    cout << "finished indexing" << endl;

    //validate(index, query_points, query_lines, query_polygons);
    benchmark(index, query_points, query_lines, query_polygons);
    
    //renderIndex(index, extent);
    delete index;
    shutdownTunaSolver();
}

void validate(spatialIndex *index, list<GIMS_Geometry *> &query_points, list<GIMS_Geometry *> &query_lines, list<GIMS_Geometry *> &query_polygons){
    //perform benchmarks
    FILE *f;
    list<GIMS_Geometry *>::iterator it;
    clock_t start, stop;

    //pt  intersects pt
    printf("VALIDATING :: pt intersects pt\n");
    f = fopen("validation_data/pt_intersects_pt","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        list<long> results = index->intersects(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pt  coveredBy  ls
    printf("VALIDATING :: pt coveredBy ls\n");
    f = fopen("validation_data/pt_coveredBy_ls","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        list<long> results = index->coveredBy(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pt  coveredBy  pol
    printf("VALIDATING :: pt coveredBy pol\n");
    f = fopen("validation_data/pt_coveredBy_pol","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        list<long> results = index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);


    //ls  contains   pt
    printf("VALIDATING :: ls contains pt\n");
    f = fopen("validation_data/ls_contains_pt","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->contains(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  covers     pt
    printf("VALIDATING :: ls covers pt\n");
    f = fopen("validation_data/ls_covers_pt","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->covers(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  contains   ls
    printf("VALIDATING :: ls contains ls\n");
    f = fopen("validation_data/ls_contains_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->contains(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  coveredBy  ls
    printf("VALIDATING :: ls coveredBy ls\n");
    f = fopen("validation_data/ls_coveredBy_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->coveredBy(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  covers     ls
    printf("VALIDATING :: ls covers ls\n");
    f = fopen("validation_data/ls_covers_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->covers(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  intersects ls
    printf("VALIDATING :: ls intersects ls\n");
    f = fopen("validation_data/ls_intersects_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->intersects(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  overlaps   ls
    printf("VALIDATING :: ls overlaps ls\n");
    f = fopen("validation_data/ls_overlaps_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->overlaps(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  touches    ls
    printf("VALIDATING :: ls touches ls\n");
    f = fopen("validation_data/ls_touches_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->touches(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);

    //ls  coveredBy  pol
    printf("VALIDATING :: ls coveredBy pol\n");
    f = fopen("validation_data/ls_coveredBy_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);

    //ls  intersects pol
    printf("VALIDATING :: ls intersects pol\n");
    f = fopen("validation_data/ls_intersects_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->intersects(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //ls  touches    pol
    printf("VALIDATING :: ls touches pol\n");
    f = fopen("validation_data/ls_touches_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        list<long> results = index->touches(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);


    //pol contains   ls
    printf("VALIDATING :: pol contains ls\n");
    f = fopen("validation_data/pol_contains_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->contains(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol contains   pol
    printf("VALIDATING :: pol contains pol\n");
    f = fopen("validation_data/pol_contains_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->contains(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol contains   pt
    printf("VALIDATING :: pol contains pt\n");
    f = fopen("validation_data/pol_contains_pt","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->contains(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol coveredBy  pol
    printf("VALIDATING :: pol coveredBy pol\n");
    f = fopen("validation_data/pol_coveredBy_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol covers     ls
    printf("VALIDATING :: pol covers ls\n");
    f = fopen("validation_data/pol_covers_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->covers(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol covers     pol
    printf("VALIDATING :: pol covers pol\n");
    f = fopen("validation_data/pol_covers_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->covers(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol covers     pt
    printf("VALIDATING :: pol covers pt\n");
    f = fopen("validation_data/pol_covers_pt","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->covers(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol intersects pol
    printf("VALIDATING :: pol intersects pol\n");
    f = fopen("validation_data/pol_intersects_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->intersects(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol overlaps   pol
    printf("VALIDATING :: pol overlaps pol\n");
    f = fopen("validation_data/pol_overlaps_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->overlaps(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol touches    ls
    printf("VALIDATING :: pol touches ls\n");
    f = fopen("validation_data/pol_touches_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->touches(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
    //pol touches    pol
    printf("VALIDATING :: pol touches pol\n");
    f = fopen("validation_data/pol_touches_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        list<long> results = index->touches(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        for( list<long>::iterator iditerator = results.begin(); iditerator != results.end(); iditerator++ ){
            GIMS_Point tmpgeom; tmpgeom.id = *iditerator;
            GIMS_Geometry *origgeom = *(idIndex.find(&tmpgeom));
            fprintf(f, "%lld | %lld\n", (*it)->osm_id, origgeom->osm_id);
        }
    }
    fclose(f);
}

void benchmark(spatialIndex *index, list<GIMS_Geometry *> &query_points, list<GIMS_Geometry *> &query_lines, list<GIMS_Geometry *> &query_polygons){
    //perform benchmarks
    list<GIMS_Geometry *>::iterator it;
    clock_t start, stop;
    FILE *f;

    //pt  intersects pt
    printf("BEGIN :: pt intersects pt\n");
    f = fopen("benchmark_data/pt_intersects_pt","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        index->intersects(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pt  coveredBy  ls
    printf("BEGIN :: pt coveredBy ls\n");
    f = fopen("benchmark_data/pt_coveredBy_ls","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        index->coveredBy(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pt  coveredBy  pol
    printf("BEGIN :: pt coveredBy pol\n");
    f = fopen("benchmark_data/pt_coveredBy_pol","w");
    for( it = query_points.begin(); it != query_points.end(); it++ ){
        start = clock();
        index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);


    //ls  contains   pt
    printf("BEGIN :: ls contains pt\n");
    f = fopen("benchmark_data/ls_contains_pt","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->contains(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  covers     pt
    printf("BEGIN :: ls covers pt\n");
    f = fopen("benchmark_data/ls_covers_pt","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->covers(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  contains   ls
    printf("BEGIN :: ls contains ls\n");
    f = fopen("benchmark_data/ls_contains_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->contains(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  coveredBy  ls
    printf("BEGIN :: ls coveredBy ls\n");
    f = fopen("benchmark_data/ls_coveredBy_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->coveredBy(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  covers     ls
    printf("BEGIN :: ls covers ls\n");
    f = fopen("benchmark_data/ls_covers_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->covers(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  intersects ls
    printf("BEGIN :: ls intersects ls\n");
    f = fopen("benchmark_data/ls_intersects_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->intersects(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  overlaps   ls
    printf("BEGIN :: ls overlaps ls\n");
    f = fopen("benchmark_data/ls_overlaps_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->overlaps(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  touches    ls
    printf("BEGIN :: ls touches ls\n");
    f = fopen("benchmark_data/ls_touches_ls","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->touches(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  coveredBy  pol
    printf("BEGIN :: ls coveredBy pol\n");
    f = fopen("benchmark_data/ls_coveredBy_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  intersects pol
    printf("BEGIN :: ls intersects pol\n");
    f = fopen("benchmark_data/ls_intersects_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->intersects(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //ls  touches    pol
    printf("BEGIN :: ls touches pol\n");
    f = fopen("benchmark_data/ls_touches_pol","w");
    for( it = query_lines.begin(); it != query_lines.end(); it++ ){
        start = clock();
        index->touches(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);


    //pol contains   ls
    printf("BEGIN :: pol contains ls\n");
    f = fopen("benchmark_data/pol_contains_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->contains(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol contains   pol
    printf("BEGIN :: pol contains pol\n");
    f = fopen("benchmark_data/pol_contains_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->contains(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol contains   pt
    printf("BEGIN :: pol contains pt\n");
    f = fopen("benchmark_data/pol_contains_pt","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->contains(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol coveredBy  pol
    printf("BEGIN :: pol coveredBy pol\n");
    f = fopen("benchmark_data/pol_coveredBy_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->coveredBy(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol covers     ls
    printf("BEGIN :: pol covers ls\n");
    f = fopen("benchmark_data/pol_covers_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->covers(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol covers     pol
    printf("BEGIN :: pol covers pol\n");
    f = fopen("benchmark_data/pol_covers_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->covers(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol covers     pt
    printf("BEGIN :: pol covers pt\n");
    f = fopen("benchmark_data/pol_covers_pt","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->covers(*it, filter_point);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol intersects pol
    printf("BEGIN :: pol intersects pol\n");
    f = fopen("benchmark_data/pol_intersects_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->intersects(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol overlaps   pol
    printf("BEGIN :: pol overlaps pol\n");
    f = fopen("benchmark_data/pol_overlaps_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->overlaps(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol touches    ls
    printf("BEGIN :: pol touches ls\n");
    f = fopen("benchmark_data/pol_touches_ls","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->touches(*it, filter_linestring);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
    //pol touches    pol
    printf("BEGIN :: pol touches pol\n");
    f = fopen("benchmark_data/pol_touches_pol","w");
    for( it = query_polygons.begin(); it != query_polygons.end(); it++ ){
        start = clock();
        index->touches(*it, filter_polygon);
        stop = clock();
        double t = (stop - start) / (CLOCKS_PER_SEC/1000.0);
        fprintf(f, "%lf\n", t);
    }
    fclose(f);
}

