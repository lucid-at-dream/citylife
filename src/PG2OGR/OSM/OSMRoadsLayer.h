/*
 * OSMRoadsLayer.h
 *
 *  Created on: Jul 6, 2014
 *      Author: carlos
 */

#ifndef OSMROADSLAYER_H_
#define OSMROADSLAYER_H_

#include "Table.h"

namespace OSM {

class OSMRoadsLayer : public Table {
public:
	OSMRoadsLayer();
	virtual ~OSMRoadsLayer();
};

} /* namespace OSM */

#endif /* OSMROADSLAYER_H_ */
