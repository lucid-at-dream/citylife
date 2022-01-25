#ifndef _DBCONNECTION_HPP_
#define _DBCONNECTION_HPP_

#include "SystemBase.hpp"
#include "Geometry.hpp"
#include "libpq-fe.h"
#include <list>
#include "conf.hpp"

using namespace std;
using namespace GIMS_GEOMETRY;

class PGConnection {
    PGconn *connection;

public:
    void connect();
    void disconnect();
    PGresult *execQuery(char *query);
    list<GIMS_Geometry *> getGeometryAsList(const char *whereClause);
    GIMS_BoundingBox *getOverallExtent();
};

/*TODO: put the following fields as configuration options*/
#define KEYWORDS                                                                                                                                               \
    { "hostaddr", "dbname", "user" }
#define VALUES                                                                                                                                                 \
    { "127.0.0.1", "portugal", "magisu" }

/* dbconnection is a connection to a postgresql database. The configurations assumed
 * are the ones specified in the configuration file.*/
extern PGConnection dbconnection;

#endif
