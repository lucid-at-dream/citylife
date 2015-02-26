#ifndef _DBCONNECTION_HPP_
#define _DBCONNECTION_HPP_

#include "libpq-fe.h"
#include <list>
#include "Geometry.hpp"

using namespace std;
using namespace GIMS_GEOMETRY;

class PGConnection{
    PGconn *connection;

  public:
    void connect();
    PGresult *execQuery(char *query);
    list<GIMS_Geometry *> *getGeometry(char *whereClause);
    GIMS_BoundingBox *getOverallExtent();
};


/*TODO: put the following fields as configuration options*/
#define SRID 3857 //projection's SRID

#define KEYWORDS {"hostaddr",  "dbname",   "user"  }
#define VALUES   {"127.0.0.1", "portugal", "magisu"}

#endif
