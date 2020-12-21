#ifndef _GIMS_GEOMETRY_HPP_
#define _GIMS_GEOMETRY_HPP_

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <string>
#include "SystemBase.hpp"
#include <iostream>
#include <list>
#include <set>
#include <gmpxx.h>
#include "conf.hpp"

using namespace std;

namespace GIMS_GEOMETRY
{
enum GeometryType {
    //envolving quad (aka envelope)
    BOUNDINGBOX = 1 << 0,
    //Elementary datatypes
    POINT = 1 << 1,
    LINESTRING = 1 << 2,
    RING = 1 << 3, //(a line string that is closed)
    POLYGON = 1 << 4,
    //Multi datatypes
    MULTIPOINT = 1 << 5,
    MULTILINESTRING = 1 << 6,
    MULTIPOLYGON = 1 << 7,
    //collections
    GEOMETRYCOLLECTION = 1 << 8,
    //auxiliary type. In fact it is a Linestring with only two points.
    LINESEGMENT = 1 << 9,
};

enum GIMS_Side {
    RIGHT = 1 << 0,
    LEFT = 1 << 1,
    ALIGNED = 1 << 2,
};

class GIMS_Geometry; //base class
class GIMS_BoundingBox;
class GIMS_Point;
class GIMS_LineString;
class GIMS_Ring;
class GIMS_Polygon;
class GIMS_MultiPoint;
class GIMS_MultiLineString;
class GIMS_MultiPolygon;
class GIMS_GeometryCollection;
class GIMS_LineSegment;

class GIMS_Geometry {
public:
    GeometryType type;
    long long id; //our id
    long long osm_id; //osm id

    //returns the subset of geometry components that intersect a given bounding box
    virtual GIMS_Geometry *clipToBox(GIMS_BoundingBox *) = 0;
    virtual GIMS_Geometry *clone() = 0;
    virtual ~GIMS_Geometry();
    virtual int getPointCount() = 0;
    virtual string toWkt() = 0;
    virtual void deleteClipped() = 0;
    virtual void deepDelete() = 0;
};

typedef struct _appr_intersection {
    bool intersects;
    double area;
} appr_intersection;

class GIMS_Approximation {
public:
    int N;
    double falsearea;
    GIMS_Point **hull;

    virtual double getArea() = 0;
    virtual double getFalseArea() = 0;

    virtual bool isDisjoint(GIMS_BoundingBox *) = 0;
    virtual bool isInside(GIMS_BoundingBox *) = 0;
    virtual bool containsPoint(GIMS_Point *) = 0;
    virtual bool containsApproximation(GIMS_Approximation *) = 0;
    virtual bool isDisjointFromApproximation(GIMS_Approximation *other) = 0;
    virtual appr_intersection intersection(GIMS_Approximation *other) = 0;

    virtual ~GIMS_Approximation();
};

class GIMS_MBNgon : public GIMS_Approximation {
public:
    bool isDisjoint(GIMS_BoundingBox *);
    bool isInside(GIMS_BoundingBox *);
    appr_intersection intersection(GIMS_Approximation *other);

    double getArea();
    double getFalseArea();

    bool containsPolygon(GIMS_Polygon *p);
    bool containsPoint(GIMS_Point *pt);
    bool containsApproximation(GIMS_Approximation *);
    bool isDisjointFromApproximation(GIMS_Approximation *other);

    GIMS_MBNgon(GIMS_Polygon *);
    ~GIMS_MBNgon();
};

class GIMS_ConvexHullAproximation : public GIMS_Approximation {
public:
    bool isDisjoint(GIMS_BoundingBox *);
    bool isInside(GIMS_BoundingBox *);
    appr_intersection intersection(GIMS_Approximation *other);

    double getArea();
    double getFalseArea();

    bool containsPolygon(GIMS_Polygon *p);
    bool containsPoint(GIMS_Point *pt);
    bool containsApproximation(GIMS_Approximation *);
    bool isDisjointFromApproximation(GIMS_Approximation *other);

    GIMS_ConvexHullAproximation(GIMS_Polygon *);
    ~GIMS_ConvexHullAproximation();
};

class GIMS_MBR : public GIMS_Approximation {
public:
    GIMS_BoundingBox *box;

    double getArea();
    double getFalseArea();
    bool isDisjoint(GIMS_BoundingBox *);
    bool isInside(GIMS_BoundingBox *);
    bool containsPoint(GIMS_Point *);
    bool containsApproximation(GIMS_Approximation *);
    bool isDisjointFromApproximation(GIMS_Approximation *other);
    appr_intersection intersection(GIMS_Approximation *other);

    GIMS_MBR(GIMS_Polygon *);
    ~GIMS_MBR();
};

class GIMS_BoundingBox : public GIMS_Geometry {
public:
    GIMS_Point *lowerLeft, *upperRight;

    string toWkt();
    bool isInside(GIMS_BoundingBox *);
    bool isDisjoint(GIMS_BoundingBox *);
    int getPointCount();
    GIMS_BoundingBox *clone();
    GIMS_Point getCenter();
    double xlength();
    double ylength();
    double minx();
    double maxx();
    double miny();
    double maxy();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void deleteClipped();
    void deepDelete();
    GIMS_BoundingBox();
    GIMS_BoundingBox(GIMS_Point *lowerLeft, GIMS_Point *upperRight);
    ~GIMS_BoundingBox();
};

class GIMS_GeometryCollection : public GIMS_Geometry {
public:
    int size;
    int allocatedSize;
    GIMS_Geometry **list;

    string toWkt();
    void append(GIMS_Geometry *);
    GIMS_GeometryCollection *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_GeometryCollection(int size);
    GIMS_GeometryCollection();
    ~GIMS_GeometryCollection();
};

class GIMS_LineSegment : public GIMS_Geometry {
public:
    GIMS_Point *p1, *p2;

    bool operator==(const GIMS_LineSegment &other) const;
    bool coversPoint(GIMS_Point *pt);
    bool isCoveredBy(std::list<GIMS_LineSegment *> &, bool copy = true);
    GIMS_Geometry *intersects(const GIMS_LineSegment *);
    string toWkt();
    GIMS_LineSegment *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    GIMS_Point closestPointWithinRange(GIMS_BoundingBox *range, GIMS_Point *pt);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_LineSegment();
    GIMS_LineSegment(GIMS_Point *p1, GIMS_Point *p2);
    ~GIMS_LineSegment();
};

class GIMS_LineString : public GIMS_Geometry {
public:
    GIMS_Point **list;
    int size;
    int allocatedSize;
    GIMS_BoundingBox bbox;

    string toWkt();
    bool isCoveredBy(std::list<GIMS_LineSegment *> &, bool copy = true);
    bool coversPoint(GIMS_Point *pt);
    GIMS_LineString *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    GIMS_LineSegment getLineSegment(int index);
    int indexOf(GIMS_Point *);
    void appendPoint(GIMS_Point *);
    void deleteClipped();
    int getPointCount();
    void deepDelete();
    GIMS_LineString(int size);
    GIMS_LineString();
    ~GIMS_LineString();
};

class GIMS_Ring : public GIMS_LineString {
public:
    GIMS_Ring(int size);
    GIMS_Ring();
};

class GIMS_MultiLineSegment : public GIMS_Geometry {
public:
    GIMS_LineSegment **list;
    int size;
    int allocatedSize;

    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    GIMS_Geometry *clone();
    int getPointCount();
    string toWkt();
    void deleteClipped();
    void deepDelete();
    void append(GIMS_LineSegment *);
    GIMS_MultiLineSegment(int size);
    GIMS_MultiLineSegment();
    ~GIMS_MultiLineSegment();
};

class GIMS_MultiLineString : public GIMS_Geometry {
public:
    GIMS_LineString **list;
    int size;
    int allocatedSize;

    string toWkt();
    int indexOf(GIMS_Point *);
    GIMS_Point *getPoint(int);
    GIMS_Point *getPrevPoint(int index);
    GIMS_Point *getNextPoint(int index);
    bool isCoveredBy(std::list<GIMS_LineSegment *> &, bool copy = true);
    bool coversPoint(GIMS_Point *pt);
    GIMS_MultiLineString *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void merge(GIMS_MultiLineString *mls);
    void append(GIMS_LineString *);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_MultiLineString(int size);
    GIMS_MultiLineString();
    ~GIMS_MultiLineString();
};

class GIMS_Point : public GIMS_Geometry {
public:
    double x, y;
    bool extractedFromDatabase;

    string toWkt();
    double distance(GIMS_Point *);
    double distanceSquared(GIMS_Point *);
    GIMS_Point getClosestPoint(GIMS_LineSegment *);
    bool equals(GIMS_Point *cmp);
    GIMS_Point *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    bool isInsideBox(GIMS_BoundingBox *box);
    bool isContainedInBox(GIMS_BoundingBox *box);
    GIMS_Side sideOf(GIMS_LineSegment *edge);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_Point();
    GIMS_Point(double x, double y, bool efd = false);
    ~GIMS_Point();
};

class GIMS_MultiPoint : public GIMS_Geometry {
public:
    int size;
    int allocatedSize;
    GIMS_Point **list;

    string toWkt();
    void append(GIMS_Point *);
    GIMS_MultiPoint *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_MultiPoint();
    GIMS_MultiPoint(int size);
    ~GIMS_MultiPoint();
};

class GIMS_Polygon : public GIMS_Geometry {
public:
    GIMS_MultiLineString *externalRing, *internalRings;

    GIMS_Approximation *approximation;
    bool isClippedCopy;
    bool hasOwnAppr;

    string toWkt();
    double area();
    GIMS_BoundingBox *computeBBox();
    GIMS_Polygon *clone();
    GIMS_BoundingBox *getExtent();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void appendExternalRing(GIMS_LineString *);
    void appendInternalRing(GIMS_LineString *);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    char containsPointWithinDomain(GIMS_Point *querypoint, GIMS_BoundingBox *domain);
    GIMS_Polygon(GIMS_MultiLineString *, GIMS_MultiLineString *, bool computebbox = true);
    GIMS_Polygon(int, int);
    GIMS_Polygon();
    ~GIMS_Polygon();
};

class GIMS_MultiPolygon : public GIMS_Geometry {
public:
    int size;
    int allocatedSize;
    GIMS_Polygon **list;

    string toWkt();
    void append(GIMS_Polygon *);
    GIMS_MultiPolygon *clone();
    GIMS_Geometry *clipToBox(GIMS_BoundingBox *);
    void deleteClipped();
    void deepDelete();
    int getPointCount();
    GIMS_MultiPolygon();
    GIMS_MultiPolygon(int size);
    ~GIMS_MultiPolygon();
};
} // namespace GIMS_GEOMETRY

using namespace GIMS_GEOMETRY;

/*Utility functions*/
double distanceSquared2p(GIMS_Point *p1, GIMS_Point *p2);
double angle3p(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3);
double cosine3p(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3);
double distToSegmentSquared(GIMS_Point *p, GIMS_LineSegment *e);
GIMS_Geometry *fromWkt(char *wkt);
GIMS_Geometry *lyWktParse(char *wkt);
int dim(GIMS_Geometry *g);
int borderDim(GIMS_Geometry *g);
bool geometryIdCmp(GIMS_Geometry *A, GIMS_Geometry *B);
bool collinear_3p(GIMS_Point *a, GIMS_Point *b, GIMS_Point *c);
int graham_scan(GIMS_Point **map, int N);
int mod(int n, int m);
double ccw(GIMS_Point *p1, GIMS_Point *p2, GIMS_Point *p3);
GIMS_Approximation *createPolygonApproximation(GIMS_Polygon *p);

typedef set<GIMS_Geometry *, decltype(&geometryIdCmp)> idset;

/* idIndex is a std::set of GIMS_Geometry* with a custom comparison function that
 * sorts the geometric objects by their ID. */
extern idset idIndex;

#endif
