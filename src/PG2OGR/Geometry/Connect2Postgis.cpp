/*
 * Connect2Postgis.cpp
 *
 *  Created on: Jun 3, 2014
 *      Author: carlos
 */

#include "Connect2Postgis.h"

namespace Geometry {

    char pgDriverConn[] = "PG:dbname=gis user=magisu hostaddr=127.0.0.1 port=5432";
    //char pgDriverConn[] = "PG:dbname=OSM user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
    //char pgDriverConn[] = "PG:dbname=OSMDB user=postgres password=postgres hostaddr=213.146.201.225 port=5432";
    //char pgDriverConn[] = "PG:dbname=OSM user=sgpgims password=gims123 hostaddr=146.185.30.61 port=5432";
    char conn[] = "dbname=gis user=magisu hostaddr=127.0.0.1 port=5432";
    //char conn[] = "dbname=OSM user=postgres password=postgres hostaddr=127.0.0.1 port=5432";
    char query[] = "select ST_AsText(way) as geom from planet_osm_line";
    
    Connect2Postgis::Connect2Postgis() {
    
        OGRRegisterAll();
        
    }
    
    Connect2Postgis::~Connect2Postgis() {
        // TODO Auto-generated destructor stub
    }
    
    OGRLayer *Connect2Postgis::GetLayerByName (char *layerName) {
    
        OGRDataSource *dataSource = OGRSFDriverRegistrar::Open (pgDriverConn, FALSE);
        
        OGRLayer *layer = dataSource->GetLayerByName (layerName);
        
        return layer;
    }
    
    OGRLineString **Connect2Postgis::GeometryArrayByDriver (int &size) {
    
        size = 0;
        
        OGRLineString **lineStringArray = NULL;
        
        OGRDataSource *dataSource = OGRSFDriverRegistrar::Open (pgDriverConn, FALSE);
        
        OGRLayer *layer = dataSource->GetLayerByName ("planet_osm_line");
        
        int countFeatures = 0;
        
        if (layer != NULL) {
            size = layer->GetFeatureCount (1);
            lineStringArray = new OGRLineString*[size];
            
            
            layer->ResetReading();
            
            OGRFeature *feature;
            
            while ( (feature = layer->GetNextFeature() ) != NULL) {
                lineStringArray[countFeatures++] = (OGRLineString *)
                                                   feature->GetGeometryRef()->clone();
                OGRFeature::DestroyFeature (feature);
            }
            
        }
        
        return lineStringArray;
    }
    
    void Connect2Postgis::DumpPolygons() {
        OGRDataSource *dataSource = OGRSFDriverRegistrar::Open (pgDriverConn, FALSE);
        
        OGRLayer *layer = dataSource->GetLayerByName ("planet_osm_polygon");
        
        OGRFeature *feature;
        
        std::cout << "[";
        
        while ( (feature = layer->GetNextFeature() ) != NULL) {
        
            OGRGeometry *geom = feature->GetGeometryRef()->clone();
            
            OGRLinearRing *extRing = ( (OGRPolygon *) geom)->getExteriorRing();
            
            OGRRawPoint *extRingPoints = (OGRRawPoint *) calloc ( extRing->getNumPoints(),
                                         sizeof (OGRRawPoint) );
                                         
            extRing->getPoints (extRingPoints);
            
            int nPoints = extRing->getNumPoints();
            
            std::cout << "[";
            
            for ( int i = 0; i < nPoints; i++ ) {
                std::cout << "[" << extRingPoints[i].x << "," << extRingPoints[i].y << "],";
            }
            
            free (extRing);
            
            std::cout << "]," << std::endl;
        }
        
        std::cout << "]" << std::endl;
    }
    
    int Connect2Postgis::CountIntersect() {
        int size = 0;
        
        OGRDataSource *dataSource = OGRSFDriverRegistrar::Open (pgDriverConn, FALSE);
        
        OGRLayer *layer = dataSource->GetLayerByName ("planet_osm_line");
        
        OGRGeometry *geom = layer->GetNextFeature()->GetGeometryRef()->clone();
        
        layer->SetSpatialFilter (geom);
        
        size = layer->GetFeatureCount (1);
        
        return size;
    }
    
    int Connect2Postgis::CountOverlaps() {
        int size = 0;
        
        OGRDataSource *dataSource = OGRSFDriverRegistrar::Open (pgDriverConn, FALSE);
        
        OGRLayer *layer = dataSource->GetLayerByName ("planet_osm_line");
        
        OGRFeature *featureToOverlap = layer->GetNextFeature();
        
        layer->ResetReading();
        
//	std::cout << "a->" <<featureToOverlap->GetFID() << std::endl;

        OGRGeometry *geom = featureToOverlap->GetGeometryRef()->clone();
        
//	OGRBoolean touch = geom->Touches(geom);

        OGREnvelope envelope;
        
        geom->getEnvelope (&envelope);
        
        layer->SetSpatialFilterRect (envelope.MinX, envelope.MinY, envelope.MaxX,
                                     envelope.MaxY);
                                     
        OGRFeature *feature;
        
        layer->ResetReading();
        
        int filtersize = layer->GetFeatureCount (1);
        
        while ( (feature = layer->GetNextFeature() ) != NULL) {
//		std::cout << feature->GetFID() << std::endl;

            if (feature->GetGeometryRef()->Touches (geom) ) {
                size++;
            }
            
            OGRFeature::DestroyFeature (feature);
        }
        
        std::cout << "a->" << filtersize << std::endl;
        
        return size;
    }
    
    
    OGRLineString **Connect2Postgis::GeometryArray (int &size) {
    
        size = 0;
        
        PGresult *result;
        //OGRGeometry *resultArray;
        OGRLineString **lineStringArray = NULL;
        
        Helper::PGConnection *pgConnection = new Helper::PGConnection (conn);
        
        if (pgConnection->Connect() ) {
            result = pgConnection->Query (query);
            
            long int numberRows = PQntuples (result);
            size = numberRows;
            
            printf ("size_pg: %d\n", size);
            
            lineStringArray = new OGRLineString*[numberRows];
            
            OGRSpatialReference *osr = NULL;
            OGRGeometry *geom = NULL;
            
            OGRSpatialReference *spatialReference = new OGRSpatialReference();
            spatialReference->importFromEPSG (900913);
            
            for (int i = 0; i < numberRows; i++) {
                char *geomValue = PQgetvalue (result, i, 0);
                OGRGeometryFactory::createFromWkt (&geomValue, osr, &geom);
                geom->assignSpatialReference (spatialReference);
                
                lineStringArray[i] = (OGRLineString *) geom->clone();
                geom->empty();
                
            }
            
            pgConnection->Close();
            
        }
        
        return lineStringArray;
    }
    
}
