#include "Geometry.hpp"
#include <iostream>

using namespace GIMS_GEOMETRY;
using namespace std;

int main(){
	GIMS_Geometry *g = fromWkt("POINT(1 1)");
	cout << g->toWkt();
	return 0;
}
