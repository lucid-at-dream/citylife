/*
 * Table.cpp
 *
 *  Created on: Jun 4, 2014
 *      Author: carlos
 */

#include "Table.h"

namespace OSM {


    OGRLayer *Table::GetLayer() {
    
        Geometry::Connect2Postgis getGeom;
        
        OGRLayer *layer = getGeom.GetLayerByName (this->name);
        
        return layer;
        
    }
    
    
}
