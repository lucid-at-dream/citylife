/*
 * PGConnection.h
 *
 *  Created on: May 26, 2014
 *      Author: carlos
 */



#ifndef PGCONNECTION_H_
#define PGCONNECTION_H_

#include "libpq-fe.h"

namespace Helper {


class PGConnection {
public:
	PGConnection();
	PGConnection(char* connStr);
	virtual ~PGConnection();
	bool Connect();
	PGresult* Query(char* queryString);
	bool Close();

private:
	PGconn* Connection;
	char* connectionString;
	PGresult* res;
	void exit_nicely(PGconn* conn);
};

} /* namespace Helper */

#endif /* PGCONNECTION_H_ */
