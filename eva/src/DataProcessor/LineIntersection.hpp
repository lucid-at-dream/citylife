#ifndef _LINEINTERSECTION_HPP_
#define _LINEINTERSECTION_HPP_

#include "Geometry.hpp"
#include "DE9IM.hpp"
#include "BentleySolver.hpp"
#include <list>

using namespace std;

void DE9IM_mls_ls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_LineString *other);
void DE9IM_mls_mls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_MultiLineString *other);

#endif