#ifndef _LINEPOLYGONINTERSECTION_HPP_
#define _LINEPOLYGONINTERSECTION_HPP_

#include "DCEL.hpp"
#include "DE9IM.hpp"
#include "Geometry.hpp"
#include "PolygonIntersection.hpp"

using namespace std;

GIMS_MultiLineString *clipMultiLineStringInDCEL(DCEL &planargraph, GIMS_BoundingBox *domain);
DCEL buildPlanarGraph_mlspol(GIMS_MultiLineString *mls, GIMS_Polygon *pol, GIMS_BoundingBox *domain);
void DE9IM_mls_pol(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_Polygon *other, GIMS_BoundingBox *domain);
void DE9IM_pol_mls(DE9IM *resultset, GIMS_Polygon *query, GIMS_MultiLineString *other, GIMS_BoundingBox *domain);
intersectionset findIntersections_mlspol(GIMS_MultiLineString *mls, GIMS_Polygon *pol);
intersectionset findIntersections_polmls(GIMS_Polygon *pol, GIMS_MultiLineString *mls);
void calculateFaceData_polmls(DCEL &dcel, GIMS_Polygon *pol, GIMS_MultiLineString *mls, GIMS_BoundingBox *domain);
void calculateFaceData_mlspol(DCEL &dcel, GIMS_MultiLineString *mls, GIMS_Polygon *pol, GIMS_BoundingBox *domain);

#endif