#ifndef GIMSGEOMETRY_HPP
#define GIMSGEOMETRY_HPP

enum GeometryType {
    BOUNDINGBOX,
    EDGE,
    POINT
};


typedef struct _GIMSBoundingBox {
    GIMSPoint *lowerLeftPt ,
              *upperRightPt;
} GIMSBoundingBox;

typedef struct _GIMSEdge {
    GIMSPoint *p1, *p2;
} GIMSEdge;

typedef struct _GIMSPoint {
    double x, y;
} GIMSPoint;


typedef union _GIMSGeometryData {
    GIMSPoint       *point;
    GIMSEdge        *edge ;
    GIMSBoundingBox *bbox ;
} GIMSGeometryData;

typedef struct _GIMSGeometry {
    GeometryType type;
    GIMSGeometryData *data;
} GIMSGeometry;

#endif
