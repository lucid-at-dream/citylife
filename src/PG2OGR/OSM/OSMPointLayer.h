/*
 * OSMPointLayer.h
 *
 *  Created on: Jul 6, 2014
 *      Author: carlos
 */

#ifndef OSMPOINTLAYER_H_
#define OSMPOINTLAYER_H_

#include "Table.h"

namespace OSM {

class OSMPointLayer : public Table {
public:
	OSMPointLayer();
	virtual ~OSMPointLayer();
};

} /* namespace OSM */

#endif /* OSMPOINTLAYER_H_ */
