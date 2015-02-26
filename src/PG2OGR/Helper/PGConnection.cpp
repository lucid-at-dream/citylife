/*
 * PGConnection.cpp
 *
 *  Created on: May 26, 2014
 *      Author: carlos
 */

#include "PGConnection.h"

namespace Helper {

    PGConnection::PGConnection() {
        // TODO Auto-generated constructor stub
    }
    
    PGConnection::PGConnection (char *connStr) {
        // TODO Auto-generated constructor stub
        connectionString = connStr;
    }
    
    bool PGConnection::Connect() {
    
        Connection = PQconnectdb (connectionString);
        
        if (PQstatus (Connection) != CONNECTION_OK) {
            exit_nicely (Connection);
            
            return false;
        }
        
        return true;
    }
    
    PGresult *PGConnection::Query (char *queryString) {
    
        res = PQexec (Connection, queryString);
        
        if (PQresultStatus (res) != PGRES_TUPLES_OK) {
            PQclear (res);
            exit_nicely (Connection);
            
            return NULL;
        }
        
        return res;
    }
    
    bool PGConnection::Close() {
    
        PQclear (res);
        exit_nicely (Connection);
        
        return true;
    }
    
    void PGConnection::exit_nicely (PGconn *conn) {
        PQfinish (conn);
    }
    
    
    PGConnection::~PGConnection() {
        // TODO Auto-generated destructor stub
        PQclear (res);
        exit_nicely (Connection);
        
    }
    
} /* namespace Helper */



