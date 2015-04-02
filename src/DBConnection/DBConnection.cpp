#include "DBConnection.hpp"

using namespace std;
using namespace GIMS_GEOMETRY;

void PGConnection::connect(){

    char *keywords[4] = KEYWORDS;
    char *values[4] = VALUES;

    this->connection = PQconnectdbParams(keywords, values, 0);

    if( PQstatus(this->connection) == CONNECTION_MADE ){
        printf("connected to the database\n");
    }else{
        printf("failed to connect\n");
    }
}

void PGConnection::disconnect(){
    PQfinish(this->connection);
}

PGresult *PGConnection::execQuery(char *query){
    return PQexec(this->connection, query);
}

AVLTree *PGConnection::getGeometry(char *whereClause){
    char buff[1000];
    sprintf(buff, "Select osm_id, st_asText(st_transform(way, %d)) %s", SRID, whereClause);
    PGresult *qres = this->execQuery(buff);

    AVLTree *resultset = new AVLTree();
    for(int i=0; i<PQntuples(qres); i++){
        long long id = atoll(PQgetvalue(qres, i, 0));
        GIMS_Geometry *g = fromWkt(PQgetvalue(qres, i, 1));
        g->id = id;
        resultset->insert(g);
    }

    PQclear(qres);

    return resultset;
}

list<GIMS_Geometry *> *PGConnection::getGeometryAsList(char *whereClause){
    char buff[1000];
    sprintf(buff, "Select osm_id, st_asText(st_transform(way, %d)) %s", SRID, whereClause);
    PGresult *qres = this->execQuery(buff);

    list<GIMS_Geometry *> *resultset = new list<GIMS_Geometry*>();
    for(int i=0; i<PQntuples(qres); i++){
        long long id = atoll(PQgetvalue(qres, i, 0));
        GIMS_Geometry *g = fromWkt(PQgetvalue(qres, i, 1));
        g->id = id;
        resultset->push_back(g);
    }

    PQclear(qres);

    return resultset;
}

GIMS_BoundingBox *PGConnection::getOverallExtent(){
    /*TODO: fetch the bounding box from the database*/
    return new GIMS_BoundingBox(new GIMS_Point(-2367875,3657494), new GIMS_Point(335642, 6671231) );
}
