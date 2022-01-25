#include "tunasolver.hpp"
#include "PMQuadTree.hpp"

PGConnection dbconnection = PGConnection();

/*inits*/
void connectToDatabase() {
    dbconnection.connect();
}

spatialIndex *createSpatialIndex(GIMS_BoundingBox *domain) {
    return new PMQUADTREE::PMQuadTree(domain);
}

int loadConfiguration(int argc, char **argv) {
    int status = conf::readConfigurationFiles(argc, argv);
    conf::printCurrentConfiguration();
    return status;
}

/*data load*/
list<GIMS_Geometry *> loadPostgisData(const char *query) {
    return dbconnection.getGeometryAsList(query);
}

/*filtering*/
int filter_bypass(GIMS_Geometry *obj) {
    return 1;
}

int filter_point(GIMS_Geometry *obj) {
    if (obj->type == POINT || obj->type == MULTIPOINT)
        return 1;
    return 0;
}

int filter_polygon(GIMS_Geometry *obj) {
    if (obj->type == POLYGON || obj->type == MULTIPOLYGON)
        return 1;
    return 0;
}

int filter_linestring(GIMS_Geometry *obj) {
    if (obj->type == LINESTRING || obj->type == MULTILINESTRING)
        return 1;
    return 0;
}

void shutdownTunaSolver() {
    conf::freeConf();
    for (idset::iterator it = idIndex.begin(); it != idIndex.end(); it++)
        (*it)->deepDelete();
}

void renderIndex(DebugRenderable *renderable, GIMS_BoundingBox *extent) {
    int argc = 1;
    char *argv[1] = { "tunasolver" };

    renderer = DebRenderer(renderable);
    renderer.setScale(400.0 / extent->xlength(), -400.0 / extent->ylength());
    renderer.setTranslation(-extent->minx(), -extent->maxy());
    renderer.mainloop(argc, argv);
}

/*topological search*/
list<long> spatialIndex::contains(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    __PMQT__verifyPolygonContainment__ = false;
    DE9IM *results = this->topologicalSearch(query, filter);
    __PMQT__verifyPolygonContainment__ = true;

    list<long> ids = results->contains();
    delete results;
    return ids;
}

list<long> spatialIndex::covers(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    __PMQT__verifyPolygonContainment__ = false;
    DE9IM *results = this->topologicalSearch(query, filter);
    __PMQT__verifyPolygonContainment__ = true;
    list<long> ids = results->covers();
    delete results;
    return ids;
}

list<long> spatialIndex::intersects(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    DE9IM *results = this->topologicalSearch(query, filter);
    list<long> ids = results->intersects();
    delete results;
    return ids;
}

list<long> spatialIndex::coveredBy(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    DE9IM *results = this->topologicalSearch(query, filter);
    list<long> ids = results->coveredBy();
    delete results;
    return ids;
}

list<long> spatialIndex::touches(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    __PMQT__verifyPolygonContainment__ = false;
    DE9IM *results = this->topologicalSearch(query, filter);
    __PMQT__verifyPolygonContainment__ = true;
    list<long> ids = results->meets();
    delete results;
    return ids;
}

list<long> spatialIndex::overlaps(GIMS_Geometry *query, int (*filter)(GIMS_Geometry *)) {
    __PMQT__verifyPolygonContainment__ = false;
    DE9IM *results = this->topologicalSearch(query, filter);
    __PMQT__verifyPolygonContainment__ = true;
    list<long> ids = results->overlaps();
    delete results;
    return ids;
}

spatialIndex::~spatialIndex() {
}
