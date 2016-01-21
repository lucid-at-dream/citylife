#include "DBConnection.hpp"

long long _gims_id_ = 1;

using namespace std;
using namespace GIMS_GEOMETRY;

void PGConnection::connect(){

    int argc = configuration.db_confs.size();

    const char *keywords[argc+1];
    const char *values[argc+1];

    keywords[argc] = values[argc] = NULL;

    int i=0;
    for(list<conf::db_conf>::iterator it = configuration.db_confs.begin(); it != configuration.db_confs.end(); it++){
        keywords[i] = it->keyword;
        values[i]   = it->value;
        i++;
    }

    for(i=0; i<argc; i++)
        cout << keywords[i] << ": " << values[i] << endl;

    this->connection = PQconnectdbParams(keywords, values, 0);

    if( PQstatus(this->connection) == CONNECTION_OK ){
        printf("Connection to database suceeded.\n");
    }else{
        cout << "Database connection error: " << PQerrorMessage(this->connection) << endl;
    }
}

void PGConnection::disconnect(){
    PQfinish(this->connection);
}

PGresult *PGConnection::execQuery(char *query){
    return PQexec(this->connection, query);
}

list<GIMS_Geometry *> PGConnection::getGeometryAsList(const char *whereClause){
    char buff[1000];
    sprintf(buff, "Select osm_id, st_asText(st_transform(way, %d)) %s", configuration.projection_srid, whereClause);
    PGresult *qres = this->execQuery(buff);

    list<GIMS_Geometry *> resultset = list<GIMS_Geometry*>();
    for(int i=0; i<PQntuples(qres); i++){
        long long id = atoll(PQgetvalue(qres, i, 0));
        GIMS_Geometry *g = fromWkt(PQgetvalue(qres, i, 1));
        g->id = _gims_id_++;
        g->osm_id = id;
        if(g->type == MULTIPOLYGON){
            g->deepDelete();
            continue;
        }
        resultset.push_back(g);
    }

    PQclear(qres);

    return resultset;
}

GIMS_BoundingBox *PGConnection::getOverallExtent(){
    /*TODO: fetch the bounding box from the database*/
    return new GIMS_BoundingBox(new GIMS_Point(-2367875,3657494), new GIMS_Point(335642, 6671231) );
}
