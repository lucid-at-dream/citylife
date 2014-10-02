/*
 * Connect2Postgis.h
 *
 *  Created on: Jun 3, 2014
 *      Author: carlos
 */

#ifndef CONNECT2POSTGIS_H_
#define CONNECT2POSTGIS_H_

#include <iostream>
#include <string>
#include "ogrsf_frmts.h"
#include "PGConnection.h"

namespace Geometry {

class Connect2Postgis {
public:
	Connect2Postgis();
	virtual ~Connect2Postgis();

	OGRLayer* GetLayerByName(char*);
	OGRLineString** GeometryArray(int&);
	OGRLineString** GeometryArrayByDriver(int&);
	int CountIntersect();
    void DumpPolygons();
	int CountOverlaps();

};

} /* namespace Helper */

#endif /* CONNECT2POSTGIS_H_ */
