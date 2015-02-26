#include <cstdio>
#include <cstdlib>
#include <iostream>
#include "DBConnection.hpp"
#include "Geometry.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"

using namespace PMQUADTREE;
using namespace GIMS_GEOMETRY;

int total = 0;
unsigned long int incId = 1;

int main (int argc, char *argv[]) {

    PGConnection conn = PGConnection();
    conn.connect();

    /*retrieve layers extent*/
    GIMS_BoundingBox *extent = conn.getOverallExtent();

    /*create an empty pmqtree bounded by the computed extent*/
    PMQuadTree *tree = new PMQuadTree( extent );

    list<GIMS_Geometry *> *polygons = conn.getGeometry("from planet_osm_polygon LIMIT 100");
    list<GIMS_Geometry *> *lines    = conn.getGeometry("from planet_osm_line LIMIT    100");
    list<GIMS_Geometry *> *points   = conn.getGeometry("from planet_osm_point LIMIT   100");
    list<GIMS_Geometry *> *roads    = conn.getGeometry("from planet_osm_roads LIMIT   100");

    tree->query = polygons->front();
    tree->query->toWkt();

    tree->insert(polygons);
    tree->insert(lines);
    tree->insert(points);
    tree->insert(roads);

    renderer = new DebRenderer();
    renderer->setScale(400.0/extent->xlength(), -400.0/extent->ylength());
    renderer->setTranslation( -extent->minx(), -extent->maxy() );
    renderer->renderCallback = tree;
    //renderer->renderSvg("outtree.svg", 400, 400);
    renderer->mainloop(argc, argv);

    return 0;
}
