/*
 * Table.h
 *
 *  Created on: Jun 4, 2014
 *      Author: carlos
 */

#ifndef TABLE_H_
#define TABLE_H_

#include "ogrsf_frmts.h"
#include "Connect2Postgis.h"

namespace OSM {

 class Table {
public:
	OGRLayer *GetLayer();

protected:
	char *name;

};


} /* namespace OSM */

#endif /* TABLE_H_ */
