#include "Geometry.hpp"

int GIMS_Polygon::getPointCount()
{
    int total = 0;
    if (this->externalRing != NULL)
        total += this->externalRing->getPointCount();
    if (this->internalRings != NULL)
        total += this->internalRings->getPointCount();
    return total;
}

void GIMS_Polygon::deepDelete()
{
    if (externalRing != NULL) {
        this->externalRing->deepDelete();
        this->externalRing = NULL;
    }
    if (internalRings != NULL) {
        this->internalRings->deepDelete();
        this->internalRings = NULL;
    }
    delete this;
}

void GIMS_Polygon::deleteClipped()
{
    if (!isClippedCopy)
        return;

    if (this->externalRing != NULL) {
        this->externalRing->deleteClipped();
        this->externalRing = NULL;
    }
    if (this->internalRings != NULL) {
        this->internalRings->deleteClipped();
        this->internalRings = NULL;
    }
    delete this;
}

GIMS_BoundingBox *GIMS_Polygon::getExtent()
{
    return this->computeBBox();
}

double GIMS_Polygon::area()
{
    double area = 0;

    if (externalRing == NULL)
        return area;

    /*add the area of each external ring*/
    for (int i = 0; i < externalRing->size; i++) {
        GIMS_Point *p1 = externalRing->list[i]->list[0];
        double determinant_sum = 0;
        for (int j = 1; j < externalRing->list[i]->size; j++) {
            GIMS_Point *p2 = externalRing->list[i]->list[j];
            double determinant = p1->x * p2->y - p2->x * p1->y;
            determinant_sum += determinant;
            p1 = p2;
        }
        double ringarea = fabs(0.5 * determinant_sum);
        area += ringarea;
    }

    if (internalRings == NULL)
        return area;

    /*subtract the area of each internal ring*/
    for (int i = 0; i < internalRings->size; i++) {
        GIMS_Point *p1 = internalRings->list[i]->list[0];
        double determinant_sum = 0;
        for (int j = 1; j < internalRings->list[i]->size; j++) {
            GIMS_Point *p2 = internalRings->list[i]->list[j];
            double determinant = p1->x * p2->y - p2->x * p1->y;
            determinant_sum += determinant;
            p1 = p2;
        }
        double ringarea = fabs(0.5 * determinant_sum);
        area -= ringarea;
    }

    return area;
}

string GIMS_Polygon::toWkt()
{
    string wkt = string("POLYGON(");
    char buff[100];
    for (int i = 0; i < this->externalRing->list[0]->size; i++) {
        if (i == 0)
            wkt += "(";
        sprintf(buff, "%lf %lf", this->externalRing->list[0]->list[i]->x, this->externalRing->list[0]->list[i]->y);
        wkt += string(buff) + (i < this->externalRing->list[0]->size - 1 ? string(",") : string(")"));
    }

    if (this->internalRings == NULL) {
        wkt += ")";
        return wkt;
    } else
        wkt += ",";

    for (int i = 0; i < this->internalRings->size; i++) {
        for (int j = 0; j < this->internalRings->list[i]->size; j++) {
            if (j == 0)
                wkt += "(";
            sprintf(buff, "%lf %lf", this->internalRings->list[i]->list[j]->x,
                    this->internalRings->list[i]->list[j]->y);
            wkt += string(buff) + (j < this->internalRings->list[i]->size - 1 ? string(",") : string(")"));
        }
        if (i < this->internalRings->size - 1)
            wkt += ",";
        else
            wkt += ")";
    }

    return wkt;
}

GIMS_Polygon *GIMS_Polygon::clone()
{
    GIMS_Polygon *fresh = new GIMS_Polygon(this->externalRing->clone(), this->internalRings->clone());
    fresh->id = this->id;
    fresh->approximation = this->approximation;
    return fresh;
}

GIMS_Geometry *GIMS_Polygon::clipToBox(GIMS_BoundingBox *box)
{
#ifndef DONTUSEAPPROXIMATIONS
    if (this->approximation) {
        if (this->approximation->isInside(box))
            return this;

        if (this->approximation->isDisjoint(box))
            return NULL;
    }
#endif

    GIMS_MultiLineString *exterior = NULL, *interior = NULL;

    if (this->externalRing != NULL)
        exterior = (GIMS_MultiLineString *)(this->externalRing->clipToBox(box));

    if (this->internalRings != NULL)
        interior = (GIMS_MultiLineString *)(this->internalRings->clipToBox(box));

    if (exterior != NULL || interior != NULL) {
        GIMS_Polygon *clipped = new GIMS_Polygon(exterior, interior, false);
        clipped->approximation = this->approximation;
        clipped->isClippedCopy = true;
        clipped->id = this->id;
        clipped->osm_id = this->osm_id;
        return clipped;
    } else
        return NULL;
}

void GIMS_Polygon::appendExternalRing(GIMS_LineString *er)
{
    if (this->externalRing == NULL)
        this->externalRing = new GIMS_MultiLineString(1);
    this->externalRing->append(er);
}

void GIMS_Polygon::appendInternalRing(GIMS_LineString *ir)
{
    if (this->internalRings == NULL)
        this->internalRings = new GIMS_MultiLineString(1);
    this->internalRings->append(ir);
}

GIMS_BoundingBox *GIMS_Polygon::computeBBox()
{
    double maxx = -1e100, minx = 1e100, maxy = -1e100, miny = 1e100;

    for (int i = 0; i < externalRing->size; i++) {
        for (int j = 0; j < externalRing->list[i]->size; j++) {
            GIMS_Point *p = externalRing->list[i]->list[j];
            if (p->x > maxx)
                maxx = p->x;
            if (p->x < minx)
                minx = p->x;
            if (p->y > maxy)
                maxy = p->y;
            if (p->y < miny)
                miny = p->y;
        }
    }
    GIMS_BoundingBox *bbox = new GIMS_BoundingBox(new GIMS_Point(minx, miny), new GIMS_Point(maxx, maxy));
    return bbox;
}

/*Returns 0 if the point is outside the polygon, 
  1 if it lies inside and 2 if it lies on the polygon's border.*/
char GIMS_Polygon::containsPointWithinDomain(GIMS_Point *querypoint, GIMS_BoundingBox *domain)
{
#ifndef DONTUSEAPPROXIMATIONS
    if (this->approximation && !this->approximation->containsPoint(querypoint))
        return 0;
#endif

    //keep track whether the closest edge is from an internal ring or the external
    //ring, as it will affect the side the point should be on to be contained.
    bool isEdgeFromExtRing = false;
    double minDist = 1e100;
    double tmp;

    GIMS_LineSegment closest;
    GIMS_Point closestPoint;

    //iterate over the edges from the polygon rings
    GIMS_MultiLineString *prings[2] = { this->externalRing, this->internalRings };

    for (GIMS_MultiLineString *ring : prings) {
        for (int i = 0; ring != NULL && i < ring->size; i++) {
            for (int j = 0; j < ring->list[i]->size - 1; j++) {
                GIMS_LineSegment curr = ring->list[i]->getLineSegment(j);
                GIMS_Point tmp_point = curr.closestPointWithinRange(domain, querypoint);

                if ((tmp = querypoint->distanceSquared(&tmp_point)) < minDist) {
                    minDist = tmp;
                    closest = curr;
                    closestPoint = tmp_point;
                    isEdgeFromExtRing = (ring == this->externalRing);
                    if (minDist < ERR_MARGIN)
                        return 2;
                }
            }
        }
    }

    /*if the closest line segment shares the node with an adjacent line segment,
      we must get the 2nd edge that shares the same endpoint.
      We can call the shared enpoint pt2.
      Given this convention, we must now check which of the two line segments 
      forms the smallest angle with the linesegment pt---pt2. We then check to 
      with side of that line pt lies.*/

    if (closestPoint.equals(closest.p1) || closestPoint.equals(closest.p2)) {
        GIMS_MultiLineString *src = isEdgeFromExtRing ? this->externalRing : this->internalRings;

        GIMS_Point *shpoint = closestPoint.equals(closest.p1) ? closest.p1 : closest.p2;
        GIMS_Point *unshared1 = closestPoint.equals(closest.p1) ? closest.p2 : closest.p1;
        GIMS_Point *unshared2 = NULL;
        GIMS_LineSegment other;

        //find the edge that shares the endpoint with current "closest"
        bool found = false;
        int i, j;
        for (i = 0; i < src->size && !found; i++) {
            for (j = 0; j < src->list[i]->size - 1 && !found; j++) {
                other = src->list[i]->getLineSegment(j);
                if (other.p1->equals(shpoint) && !other.p2->equals(unshared1)) {
                    unshared2 = other.p2;
                    found = true;
                } else if (other.p2->equals(shpoint) && !other.p1->equals(unshared1)) {
                    unshared2 = other.p1;
                    found = true;
                }
            }
        }

        //compute and compare angles
        double angle1 = angle3p(unshared1, shpoint, querypoint);
        double angle2 = (unshared2 != NULL) ? angle3p(unshared2, shpoint, querypoint) : 1e100;

        closest = angle1 < angle2 ? closest : other;
    }

    /*Finally we check to which side of "closest" "pt" lies and report.*/
    GIMS_Side side = querypoint->sideOf(&closest);

    switch (side) {
    case LEFT:
        return 0;
    case RIGHT:
        return 1;
    default:
        return 2;
    }
}

GIMS_Polygon::GIMS_Polygon(GIMS_MultiLineString *externalRing, GIMS_MultiLineString *internalRings,
                           bool computeApproximation)
{
    this->type = POLYGON;
    this->hasOwnAppr = false;
    this->isClippedCopy = false;
    this->externalRing = externalRing;
    this->internalRings = internalRings;
    if (computeApproximation) {
        this->hasOwnAppr = true;
        this->approximation = createPolygonApproximation(this);
    }
}

GIMS_Polygon::GIMS_Polygon(int ext_alloc, int int_alloc)
{
    this->id = 0;
    this->type = POLYGON;
    this->hasOwnAppr = false;
    this->isClippedCopy = false;
    this->externalRing = new GIMS_MultiLineString(ext_alloc);
    this->internalRings = new GIMS_MultiLineString(int_alloc);
    this->approximation = NULL;
}

GIMS_Polygon::GIMS_Polygon()
{
    this->id = 0;
    this->type = POLYGON;
    this->hasOwnAppr = false;
    this->isClippedCopy = false;
    this->externalRing = this->internalRings = NULL;
    this->approximation = NULL;
}

GIMS_Polygon::~GIMS_Polygon()
{
    if (this->externalRing)
        delete this->externalRing;
    if (this->internalRings)
        delete this->internalRings;
    if (this->hasOwnAppr && this->approximation)
        delete this->approximation;
}

int GIMS_MultiPolygon::getPointCount()
{
    int total = 0;
    for (int i = 0; i < size; i++)
        total += list[i]->getPointCount();
    return total;
}

void GIMS_MultiPolygon::deepDelete()
{
    if (this->list != NULL)
        for (int i = 0; i < this->size; i++)
            this->list[i]->deepDelete();
    delete this;
}

void GIMS_MultiPolygon::deleteClipped()
{
    for (int i = 0; i < this->size; i++) {
        this->list[i]->deleteClipped();
    }
    delete this;
}

void GIMS_MultiPolygon::append(GIMS_Polygon *p)
{
    this->size++;
    if (this->size > this->allocatedSize) {
        this->list = (GIMS_Polygon **)realloc(this->list, this->size * sizeof(GIMS_Polygon *));
        this->allocatedSize = size;
    }
    this->list[size - 1] = p;
}

string GIMS_MultiPolygon::toWkt()
{
    string wkt = string("MULTIPOLYGON(");
    char buff[100];
    for (int k = 0; k < this->size; k++) {
        wkt += "(";
        GIMS_Polygon *pol = this->list[k];
        for (int i = 0; i < pol->externalRing->list[0]->size; i++) {
            if (i == 0)
                wkt += "(";
            sprintf(buff, "%lf %lf", pol->externalRing->list[0]->list[i]->x, pol->externalRing->list[0]->list[i]->y);
            wkt += string(buff) + (i < pol->externalRing->list[0]->size - 1 ? string(",") : string(")"));
        }

        if (pol->internalRings == NULL) {
            wkt += ")";
        } else {
            wkt += ",";
            for (int i = 0; i < pol->internalRings->size; i++) {
                for (int j = 0; j < pol->internalRings->list[i]->size; j++) {
                    if (j == 0)
                        wkt += "(";
                    sprintf(buff, "%lf %lf", pol->internalRings->list[i]->list[j]->x,
                            pol->internalRings->list[i]->list[j]->y);
                    wkt += string(buff) + (j < pol->internalRings->list[i]->size - 1 ? string(",") : string(")"));
                }
                wkt += i < pol->internalRings->size - 1 ? "," : ")";
            }
        }

        wkt += k < this->size - 1 ? "," : ")";
    }

    return wkt;
}

GIMS_MultiPolygon *GIMS_MultiPolygon::clone()
{
    GIMS_MultiPolygon *fresh = new GIMS_MultiPolygon(this->allocatedSize);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_Polygon *));
    fresh->size = this->size;
    fresh->id = this->id;
    return fresh;
}

GIMS_Geometry *GIMS_MultiPolygon::clipToBox(GIMS_BoundingBox *box)
{
    if (this->list == NULL)
        return NULL;

    GIMS_MultiPolygon *clipped = NULL;

    for (int i = 0; i < this->size; i++) {
        GIMS_Polygon *partial = (GIMS_Polygon *)(this->list[i]->clipToBox(box));
        if (partial != NULL) {
            if (clipped == NULL) {
                clipped = new GIMS_MultiPolygon(1);
                clipped->id = this->id;
            }
            clipped->append(partial);
        }
    }
    return clipped;
}

GIMS_MultiPolygon::GIMS_MultiPolygon()
{
    this->id = 0;
    this->type = MULTIPOLYGON;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
}

GIMS_MultiPolygon::GIMS_MultiPolygon(int size)
{
    this->type = MULTIPOLYGON;
    this->id = 0;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_Polygon **)malloc(size * sizeof(GIMS_Polygon *));
}

GIMS_MultiPolygon::~GIMS_MultiPolygon()
{
    if (this->list)
        free(this->list);
}
