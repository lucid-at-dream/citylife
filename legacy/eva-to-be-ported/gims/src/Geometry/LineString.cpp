#include "Geometry.hpp"

bool GIMS_LineSegment::operator==(const GIMS_LineSegment &other) const
{
    if (this->p1->equals(other.p1) && this->p2->equals(other.p2))
        return true;
    return false;
}

int GIMS_LineSegment::getPointCount()
{
    return 2;
}

void GIMS_LineSegment::deleteClipped()
{
}

void GIMS_LineSegment::deepDelete()
{
    delete this->p1;
    delete this->p2;
    delete this;
}

string GIMS_LineSegment::toWkt()
{
    char buff[100];
    sprintf(buff, "LINESTRING(%lf %lf, %lf %lf)", this->p1->x, this->p1->y, this->p2->x, this->p2->y);
    return buff;
}

GIMS_LineSegment *GIMS_LineSegment::clone()
{
    GIMS_LineSegment *fresh = new GIMS_LineSegment(this->p1->clone(), this->p2->clone());
    fresh->id = this->id;
    return fresh;
}

GIMS_LineSegment::GIMS_LineSegment()
{
    this->type = LINESEGMENT;
    this->id = 0;
}

GIMS_LineSegment::GIMS_LineSegment(GIMS_Point *p1, GIMS_Point *p2)
{
    this->id = 0;
    this->type = LINESEGMENT;
    this->p1 = p1;
    this->p2 = p2;
}

bool GIMS_LineSegment::isCoveredBy(std::list<GIMS_LineSegment *> &linesegments, bool copy)
{
    return false;
}

bool GIMS_LineSegment::coversPoint(GIMS_Point *pt)
{
    double miny = p2->y < p1->y ? p2->y : p1->y, maxy = p2->y > p1->y ? p2->y : p1->y,
           minx = p2->x < p1->x ? p2->x : p1->x, maxx = p2->x > p1->x ? p2->x : p1->x;

    /*if line is vertical*/
    if (p1->x == p2->x) {
        if (pt->x == p1->x && pt->y <= maxy + ERR_MARGIN && pt->y >= miny - ERR_MARGIN)
            return true;
        return false;
    }

    /*if line is horizontal*/
    if (p1->y == p2->y) {
        if (pt->y == p1->y && pt->x <= maxx + ERR_MARGIN && pt->x >= minx - ERR_MARGIN)
            return true;
        return false;
    }

    double m = (p2->y - p1->y) / (p2->x - p1->x);
    double b = p1->y - m * p1->x;
    if (fabs(pt->y - (m * pt->x + b)) >= ERR_MARGIN) {
        return false;
    }

    if (pt->y <= maxy + ERR_MARGIN && pt->y >= miny - ERR_MARGIN)
        return true;
    return false;
}

GIMS_LineSegment::~GIMS_LineSegment()
{
}

/*returns the point contained in this line segment that is both inside the give
  bounding box "range" and is closer to point "pt" than any other point in the
  line segment. it is assumed that the line segment allways has some portion of 
  it inside the given range.*/
GIMS_Point GIMS_LineSegment::closestPointWithinRange(GIMS_BoundingBox *range, GIMS_Point *pt)
{
    bool p1inside = this->p1->isInsideBox(range), p2inside = this->p2->isInsideBox(range);

    /*if this line segment is fully within the given range we can solve the 
      unconstrained version of the problem.*/
    if (p1inside && p2inside)
        return pt->getClosestPoint(this);

    /*otherwise we must calculate the clipped line segment and use it as input to
      the unconstrained version of the problem.*/

    GIMS_LineSegment clipped;
    GIMS_Point p1, p2;

    /*these are the square limits*/
    double ymax = range->upperRight->y, ymin = range->lowerLeft->y, xmax = range->upperRight->x,
           xmin = range->lowerLeft->x;

    /*if line is vertical*/
    if (this->p1->x == this->p2->x) {
        if (p1inside) {
            double int_y = this->p1->y > this->p2->y ? ymin : ymax;
            p1 = *(this->p1);
            p2 = GIMS_Point(this->p1->x, int_y);
        } else if (p2inside) {
            double int_y = this->p2->y > this->p1->y ? ymin : ymax;
            p1 = GIMS_Point(this->p1->x, int_y);
            p2 = *(this->p2);
        } else {
            p1 = GIMS_Point(this->p1->x, ymin);
            p2 = GIMS_Point(this->p1->x, ymax);
        }

        /*if line is horizontal*/
    } else if (this->p1->y == this->p2->y) {
        if (p1inside) {
            double int_x = this->p1->x > this->p2->x ? xmin : xmax;
            p1 = *(this->p1);
            p2 = GIMS_Point(int_x, this->p1->y);

        } else if (p2inside) {
            double int_x = this->p2->x > this->p1->x ? xmin : xmax;
            p1 = GIMS_Point(int_x, this->p1->y);
            p2 = *(this->p2);

        } else {
            p1 = GIMS_Point(xmin, this->p1->y);
            p2 = GIMS_Point(xmax, this->p1->y);
        }

        /*given that we've tested for both vertical and horizontal lines, we may now
      assume that the line segment has a declive different from zero.*/
    } else {
        /*for each line that forms the bounding box, compute the value of t in
          the line equation l(t) = p1 + t*(p2-p1), that results in an intersection.*/

        double t_int_ymax = (ymax - this->p1->y) / (this->p2->y - this->p1->y),
               t_int_ymin = (ymin - this->p1->y) / (this->p2->y - this->p1->y),
               t_int_xmax = (xmax - this->p1->x) / (this->p2->x - this->p1->x),
               t_int_xmin = (xmin - this->p1->x) / (this->p2->x - this->p1->x);

        /*there are at most two of this values of t within the range [0,1], that
          is, contained in the line segment. There's also at least one within 
          that range because otherwise both line segment endpoints would be 
          contained in the bounding box and we already ruled out that case.*/

        /*we now compute the clipped line segment endpoints. One of them may be
          a line segment endpoint itself. Therefore we initialize accordingly.*/
        if (p1inside)
            p1 = *(this->p1);
        else if (p2inside)
            p1 = *(this->p2);
        bool p1found = p1inside || p2inside;

        /*For each value of "t", we check if the corresponding point is within 
          the given range. Also, to prevent double counting intersection points
          that lie in the square's corner we check if a similar point hasn't 
          been counted*/
        double t_values[] = { t_int_ymax, t_int_ymin, t_int_xmax, t_int_xmin };
        for (double t : t_values) {
            //outside the line segment.
            if (t < 0 || t > 1)
                continue;

            GIMS_Point ipt = GIMS_Point(this->p1->x + t * (this->p2->x - this->p1->x),
                                        this->p1->y + t * (this->p2->y - this->p1->y));

            //the resulting point is outside the given range.
            if (ipt.x > xmax || ipt.x < xmin || ipt.y > ymax || ipt.y < ymin)
                continue;

            //right now we now that the resulting point is a valid intersection
            //point. Never the less we still have to figure out if it's been
            //previously counted or not.
            if (!p1found) {
                p1 = ipt;
                p1found = true;
            } else if (!ipt.equals(&p1))
                p2 = ipt;
        }
    }

    clipped = GIMS_LineSegment(&p1, &p2);
    return pt->getClosestPoint(&clipped);
}

GIMS_Geometry *GIMS_LineSegment::intersects(const GIMS_LineSegment *other)
{
    if (this->p1 == NULL || this->p2 == NULL) {
        printf("got NULL points\n");
        return NULL;
    }

    double int_x, int_y;

    //if this is a vertical line
    if (this->p1->x == this->p2->x) {
        int_x = this->p1->x;

        //if other is vertical
        if (other->p1->x == other->p2->x) {
            if (other->p1->x != this->p1->x)
                return NULL;

            //Is there an overlap in the y axis?
            double top_this = MAX(this->p1->y, this->p2->y), bottom_this = MIN(this->p1->y, this->p2->y),
                   top_other = MAX(other->p1->y, other->p2->y), bottom_other = MIN(other->p1->y, other->p2->y);

            //the linesegments simply touch
            if (top_other < bottom_this + ERR_MARGIN && top_other > bottom_this - ERR_MARGIN)
                return new GIMS_Point(int_x, bottom_this);
            if (top_this < bottom_other + ERR_MARGIN && top_this > bottom_other - ERR_MARGIN)
                return new GIMS_Point(int_x, bottom_other);

            //if there's no overlap
            if (top_this < bottom_other - ERR_MARGIN || top_other < bottom_this - ERR_MARGIN)
                return NULL;
            else
                return new GIMS_LineSegment(new GIMS_Point(int_x, MIN(top_this, top_other)),
                                            new GIMS_Point(int_x, MAX(bottom_this, bottom_other)));

        }

        //if other is horizontal
        else if (other->p1->y == other->p2->y) {
            int_y = other->p1->y;

            if (int_x <= MAX(other->p1->x, other->p2->x) + ERR_MARGIN &&
                int_x >= MIN(other->p1->x, other->p2->x) - ERR_MARGIN &&
                int_y <= MAX(this->p1->y, this->p2->y) + ERR_MARGIN &&
                int_y >= MIN(this->p1->y, this->p2->y) - ERR_MARGIN)
                return new GIMS_Point(int_x, int_y);
            return NULL;
        }

        //if other is neither vertical nor horizontal
        else {
            double m = (other->p2->y - other->p1->y) / (other->p2->x - other->p1->x),
                   b = other->p1->y - m * other->p1->x;

            int_y = m * int_x + b;

            if (int_x <= MAX(other->p1->x, other->p2->x) + ERR_MARGIN &&
                int_x >= MIN(other->p1->x, other->p2->x) - ERR_MARGIN &&
                int_y <= MAX(this->p1->y, this->p2->y) + ERR_MARGIN &&
                int_y >= MIN(this->p1->y, this->p2->y) - ERR_MARGIN)
                return new GIMS_Point(int_x, int_y);
            return NULL;
        }
    }

    //if this line is horizontal
    if (this->p1->y == this->p2->y) {
        int_y = this->p1->y;

        //if other is vertical
        if (other->p1->x == other->p2->x) {
            int_x = other->p1->x;

            if (int_x <= MAX(this->p1->x, this->p2->x) + ERR_MARGIN &&
                int_x >= MIN(this->p1->x, this->p2->x) - ERR_MARGIN &&
                int_y <= MAX(other->p1->y, other->p2->y) + ERR_MARGIN &&
                int_y >= MIN(other->p1->y, other->p2->y) - ERR_MARGIN)
                return new GIMS_Point(int_x, int_y);
            return NULL;

        }

        //if other is horizontal
        else if (other->p1->y == other->p2->y) {
            if (other->p1->y != this->p1->y)
                return NULL;

            //Is there an overlap in the y axis?
            double right_this = MAX(this->p1->x, this->p2->x), left_this = MIN(this->p1->x, this->p2->x),
                   right_other = MAX(other->p1->x, other->p2->x), left_other = MIN(other->p1->x, other->p2->x);

            //the linesegments simply touch
            if (right_other < left_this + ERR_MARGIN && right_other > left_this - ERR_MARGIN)
                return new GIMS_Point(left_this, int_y);
            if (right_this < left_other + ERR_MARGIN && right_this > left_other - ERR_MARGIN)
                return new GIMS_Point(left_other, int_y);

            //if there's no overlap
            if (right_this < left_other + ERR_MARGIN || right_other < left_this + ERR_MARGIN)
                return NULL;
            else
                return new GIMS_LineSegment(new GIMS_Point(MIN(right_this, right_other), int_y),
                                            new GIMS_Point(MAX(left_this, left_other), int_y));

        }

        //if other is neither vertical nor horizontal
        else {
            double m = (other->p2->y - other->p1->y) / (other->p2->x - other->p1->x),
                   b = other->p1->y - m * other->p1->x;

            int_x = (int_y - b) / m;

            if (int_x <= MAX(other->p1->x, other->p2->x) + ERR_MARGIN &&
                int_x >= MIN(other->p1->x, other->p2->x) - ERR_MARGIN &&
                int_x <= MAX(this->p1->x, this->p2->x) + ERR_MARGIN &&
                int_x >= MIN(this->p1->x, this->p2->x) - ERR_MARGIN) {
                return new GIMS_Point(int_x, int_y);
            }
            return NULL;
        }
    }

    //if the other line is vertical and this is neither vertical nor horizontal
    if (other->p1->x == other->p2->x) {
        int_x = other->p1->x;

        double m = (this->p2->y - this->p1->y) / (this->p2->x - this->p1->x), b = this->p1->y - m * this->p1->x;

        int_y = m * int_x + b;

        if (int_x <= MAX(this->p1->x, this->p2->x) + ERR_MARGIN &&
            int_x >= MIN(this->p1->x, this->p2->x) - ERR_MARGIN &&
            int_y <= MAX(other->p1->y, other->p2->y) + ERR_MARGIN &&
            int_y >= MIN(other->p1->y, other->p2->y) - ERR_MARGIN)
            return new GIMS_Point(int_x, int_y);
        return NULL;
    }

    //if the other line is horizontal and this is neither vertical nor horizontal
    if (other->p1->y == other->p2->y) {
        int_y = other->p1->y;

        double m = (this->p2->y - this->p1->y) / (this->p2->x - this->p1->x), b = this->p1->y - m * this->p1->x;

        int_x = (int_y - b) / m;

        if (int_x <= MAX(this->p1->x, this->p2->x) + ERR_MARGIN &&
            int_x >= MIN(this->p1->x, this->p2->x) - ERR_MARGIN &&
            int_x <= MAX(other->p1->x, other->p2->x) + ERR_MARGIN &&
            int_x >= MIN(other->p1->x, other->p2->x) - ERR_MARGIN) {
            return new GIMS_Point(int_x, int_y);
        } else {
            return NULL;
        }
    }

    //if both lines are neither horizontal nor vertical
    double m1 = (this->p2->y - this->p1->y) / (this->p2->x - this->p1->x), b1 = this->p1->y - m1 * this->p1->x;

    double m2 = (other->p2->y - other->p1->y) / (other->p2->x - other->p1->x), b2 = other->p1->y - m2 * other->p1->x;

    //if the lines are parallel
    if (m1 == m2) {
        //if these are segments from the same line
        if (b1 == b2) {
            //Is there an overlap in the x axis?
            double right_this = MAX(this->p1->x, this->p2->x), left_this = MIN(this->p1->x, this->p2->x),
                   right_other = MAX(other->p1->x, other->p2->x), left_other = MIN(other->p1->x, other->p2->x);

            //the linesegments simply touch
            if (right_other < left_this + ERR_MARGIN && right_other > left_this - ERR_MARGIN)
                return new GIMS_Point(left_this, m1 * left_this + b1);
            if (right_this < left_other + ERR_MARGIN && right_this > left_other - ERR_MARGIN)
                return new GIMS_Point(left_other, m1 * left_other + b1);

            //if there's no overlap
            if (right_this < left_other + ERR_MARGIN || right_other < left_this + ERR_MARGIN)
                return NULL;
            else {
                double xi = MAX(left_this, left_other), xf = MIN(right_this, right_other);
                return new GIMS_LineSegment(new GIMS_Point(xi, m1 * xi + b1), new GIMS_Point(xf, m1 * xf + b1));
            }
        }
        return NULL;
    }

#define PRECISION 10000000.0

    mpf_class this_p1_x = this->p1->x, this_p1_y = this->p1->y, this_p2_x = this->p2->x, this_p2_y = this->p2->y,
              other_p1_x = other->p1->x, other_p1_y = other->p1->y, other_p2_x = other->p2->x,
              other_p2_y = other->p2->y;

    mpf_class s1_x, s1_y, s2_x, s2_y;
    s1_x = this_p2_x - this_p1_x;
    s1_y = this_p2_y - this_p1_y;
    s2_x = other_p2_x - other_p1_x;
    s2_y = other_p2_y - other_p1_y;

    mpf_class s_den = (-s2_x * s1_y + s1_x * s2_y),
              s_num = (-s1_y * (this_p1_x - other_p1_x) + s1_x * (this_p1_y - other_p1_y)),
              t_den = (-s2_x * s1_y + s1_x * s2_y),
              t_num = (s2_x * (this_p1_y - other_p1_y) - s2_y * (this_p1_x - other_p1_x));

    mpf_class s = (s_num / s_den), t = (t_num / t_den);

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        mpf_class fx = this->p1->x + (t * s1_x), fy = this->p1->y + (t * s1_y);

        int_x = fx.get_d();
        int_y = fy.get_d();

        return new GIMS_Point(int_x, int_y);
    }
    return NULL;
}

GIMS_Geometry *GIMS_LineSegment::clipToBox(GIMS_BoundingBox *box)
{
    GIMS_Point upperLeft = GIMS_Point(box->lowerLeft->x, box->upperRight->y),
               lowerRight = GIMS_Point(box->upperRight->x, box->lowerLeft->y);

    GIMS_Point *squarePoints[] = { &upperLeft, box->upperRight, &lowerRight, box->lowerLeft };

    /*If all square's points lie on the same side of the line defined by the
      edge's two endpoints, then there's no intersection*/
    bool allOnSameSide = true;
    double prev = squarePoints[0]->sideOf(this);
    for (int i = 1; i < 4; i++) {
        if (squarePoints[i]->sideOf(this) != prev) {
            allOnSameSide = false;
            break;
        }
    }
    if (allOnSameSide) {
        return NULL;
    }

    /*this part is reached only if the line defined by the two linesegment's endpoints
      intersects the square. Therefore, we do a projection on both the x and y axis 
      of both the edge and the square and check for overlapings*/
    if (this->p1->x > lowerRight.x && this->p2->x > lowerRight.x) {
        /*edge is to the right of the rectangle*/
        return NULL;
    }
    if (this->p1->x < upperLeft.x && this->p2->x < upperLeft.x) {
        /*edge is to the left of the rectangle*/
        return NULL;
    }
    if (this->p1->y > upperLeft.y && this->p2->y > upperLeft.y) {
        /*edge is above of the rectangle*/
        return NULL;
    }
    if (this->p1->y < lowerRight.y && this->p2->y < lowerRight.y) {
        /*edge is above of the rectangle*/
        return NULL;
    }

    return this;
}

/* From here we define the LineString class */
int GIMS_LineString::getPointCount()
{
    return size;
}

void GIMS_LineString::deepDelete()
{
    if (this->list != NULL)
        for (int i = 0; i < this->size; i++)
            delete this->list[i];
    delete this;
}

void GIMS_LineString::deleteClipped()
{
    delete this;
}

bool GIMS_LineString::coversPoint(GIMS_Point *pt)
{
    for (int i = 0; i < this->size - 1; i++) {
        if (this->getLineSegment(i).coversPoint(pt))
            return true;
    }
    return false;
}

GIMS_LineString::GIMS_LineString(int size)
{
    this->type = LINESTRING;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
    this->allocatedSize = size;
    this->size = 0;
    this->id = 0;
    this->bbox.lowerLeft = new GIMS_Point(1e10, 1e10);
    this->bbox.upperRight = new GIMS_Point(-1e10, -1e10);
}

GIMS_LineString::GIMS_LineString()
{
    this->type = LINESTRING;
    this->list = NULL;
    this->allocatedSize = 0;
    this->size = 0;
    this->id = 0;
    this->bbox.lowerLeft = new GIMS_Point(1e10, 1e10);
    this->bbox.upperRight = new GIMS_Point(-1e10, -1e10);
}

GIMS_LineString::~GIMS_LineString()
{
    if (this->list != NULL)
        free(this->list);
    delete this->bbox.lowerLeft;
    delete this->bbox.upperRight;
}

int GIMS_LineString::indexOf(GIMS_Point *p)
{
    for (int i = 0; i < this->size; i++) {
        if (p->equals(this->list[i]))
            return i;
    }
    return -1;
}

string GIMS_LineString::toWkt()
{
    string wkt = string("LINESTRING(");
    char buff[100];
    for (int i = 0; i < this->size; i++) {
        sprintf(buff, "%lf %lf", this->list[i]->x, this->list[i]->y);
        wkt += string(buff) + (i < this->size - 1 ? string(",") : string(")"));
    }
    return wkt;
}

GIMS_LineString *GIMS_LineString::clone()
{
    GIMS_LineString *newList = new GIMS_LineString(this->allocatedSize);
    newList->size = this->size;
    memcpy(newList->list, this->list, this->size * sizeof(GIMS_Point *));
    newList->id = this->id;
    return newList;
}

bool GIMS_LineString::isCoveredBy(std::list<GIMS_LineSegment *> &linesegments, bool copy)
{
    return false;
}

GIMS_Geometry *GIMS_LineString::clipToBox(GIMS_BoundingBox *box)
{
    if (box->isDisjoint(&bbox))
        return NULL;

    GIMS_MultiLineString *clipped = NULL;
    GIMS_LineString *partial = NULL;

    for (int i = 0; i < this->size - 1; i++) {
        GIMS_LineSegment segment = this->getLineSegment(i);
        if (segment.clipToBox(box) != NULL) {
            if (partial == NULL) {
                partial = new GIMS_LineString(2);
                partial->id = this->id;
                partial->appendPoint(segment.p1);
            }
            partial->appendPoint(segment.p2);
        } else if (partial != NULL) {
            if (clipped == NULL) {
                clipped = new GIMS_MultiLineString(1);
                clipped->id = this->id;
            }
            clipped->append(partial);
            partial = NULL;
        }
    }

    if (partial != NULL) {
        if (clipped == NULL) {
            clipped = new GIMS_MultiLineString(1);
            clipped->id = this->id;
        }
        clipped->append(partial);
    }

    return clipped;
}

GIMS_LineSegment GIMS_LineString::getLineSegment(int index)
{
    GIMS_LineSegment ls = GIMS_LineSegment(this->list[index], this->list[index + 1]);
    ls.id = this->id;
    return ls;
}

void GIMS_LineString::appendPoint(GIMS_Point *p)
{
    this->size += 1;

    //update bounding box
    if (p->x < bbox.lowerLeft->x)
        bbox.lowerLeft->x = p->x;
    if (p->y < bbox.lowerLeft->y)
        bbox.lowerLeft->y = p->y;
    if (p->x > bbox.upperRight->x)
        bbox.upperRight->x = p->x;
    if (p->y > bbox.upperRight->y)
        bbox.upperRight->y = p->y;

    if (this->size > this->allocatedSize) {
        this->list = (GIMS_Point **)realloc(this->list, sizeof(GIMS_Point *) * (this->size));
        this->allocatedSize = this->size;
    }

    this->list[this->size - 1] = p;
}

/*The ring class. We can take advantage of everything that was defined for LineString*/

GIMS_Ring::GIMS_Ring(int size)
{
    this->type = RING;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
    this->id = 0;
}

GIMS_Ring::GIMS_Ring()
{
    this->type = RING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
    this->id = 0;
}

GIMS_Geometry *GIMS_MultiLineSegment::clipToBox(GIMS_BoundingBox *)
{
    return NULL;
}

GIMS_Geometry *GIMS_MultiLineSegment::clone()
{
    return NULL;
}

int GIMS_MultiLineSegment::getPointCount()
{
    return size * 2;
}

string GIMS_MultiLineSegment::toWkt()
{
    string wkt = string("MULTILINESTRING(");
    char buff[100];

    for (int i = 0; i < this->size; i++) {
        sprintf(buff, "(%lf %lf, %lf %lf)", this->list[i]->p1->x, this->list[i]->p1->y, this->list[i]->p2->x,
                this->list[i]->p2->y);
        wkt += string(buff);
        wkt += i < this->size - 1 ? "," : ")";
    }

    return wkt;
}

void GIMS_MultiLineSegment::deleteClipped()
{
    for (int i = 0; i < size; i++)
        list[i]->deleteClipped();
    delete this;
}
void GIMS_MultiLineSegment::deepDelete()
{
    for (int i = 0; i < size; i++)
        list[i]->deepDelete();
    delete this;
}

void GIMS_MultiLineSegment::append(GIMS_LineSegment *l)
{
    this->size += 1;
    if (this->size > this->allocatedSize) {
        this->list = (GIMS_LineSegment **)realloc(this->list, this->size * sizeof(GIMS_LineSegment *));
        this->allocatedSize = size;
    }
    this->list[this->size - 1] = l;
}

GIMS_MultiLineSegment::GIMS_MultiLineSegment(int size)
{
    this->type = MULTILINESTRING;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_LineSegment **)malloc(size * sizeof(GIMS_LineSegment *));
    this->id = 0;
}

GIMS_MultiLineSegment::GIMS_MultiLineSegment()
{
    this->type = MULTILINESTRING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
    this->id = 0;
}

GIMS_MultiLineSegment::~GIMS_MultiLineSegment()
{
    if (list != NULL)
        free(list);
}

/*The MultiLineString class.*/
int GIMS_MultiLineString::getPointCount()
{
    int total = 0;
    for (int i = 0; i < size; i++)
        total += list[i]->size;
    return total;
}

bool GIMS_MultiLineString::coversPoint(GIMS_Point *pt)
{
    for (int i = 0; i < this->size; i++) {
        if (this->list[i]->coversPoint(pt))
            return true;
    }
    return false;
}

void GIMS_MultiLineString::deepDelete()
{
    if (this->list != NULL)
        for (int i = 0; i < this->size; i++)
            this->list[i]->deepDelete();
    delete this;
}

void GIMS_MultiLineString::deleteClipped()
{
    if (this->list != NULL)
        for (int i = 0; i < this->size; i++) {
            this->list[i]->deleteClipped();
        }
    delete this;
}

string GIMS_MultiLineString::toWkt()
{
    string wkt = string("MULTILINESTRING(");
    char buff[100];

    for (int i = 0; i < this->size; i++) {
        for (int j = 0; j < this->list[i]->size; j++) {
            if (j == 0)
                wkt += string("(");
            sprintf(buff, "%lf %lf", this->list[i]->list[j]->x, this->list[i]->list[j]->y);
            wkt += string(buff) + (j < this->list[i]->size - 1 ? string(",") : string(")"));
        }
        wkt += i < this->size - 1 ? "," : ")";
    }

    return wkt;
}

GIMS_MultiLineString *GIMS_MultiLineString::clone()
{
    GIMS_MultiLineString *fresh = new GIMS_MultiLineString(this->size);
    memcpy(fresh->list, this->list, this->size * sizeof(GIMS_LineString *));
    fresh->id = this->id;
    return fresh;
}

bool GIMS_MultiLineString::isCoveredBy(std::list<GIMS_LineSegment *> &linesegments, bool copy)
{
    return false;
}

int GIMS_MultiLineString::indexOf(GIMS_Point *p)
{
    int index = 0, rel;
    for (int i = 0; i < this->size; i++) {
        if ((rel = this->list[i]->indexOf(p)) > -1)
            return index + rel;
        else
            index += this->list[i]->size;
    }
    return -1;
}

GIMS_Point *GIMS_MultiLineString::getPoint(int index)
{
    for (int i = 0; i < this->size; i++) {
        if (index < this->list[i]->size)
            return this->list[i]->list[index];
        else
            index -= this->list[i]->size;
    }
    return NULL;
}

GIMS_Point *GIMS_MultiLineString::getNextPoint(int index)
{
    for (int i = 0; i < this->size; i++) {
        if (index < this->list[i]->size)
            if (index + 1 < this->list[i]->size)
                return this->list[i]->list[index + 1];
            else
                return NULL;
        else
            index -= this->list[i]->size;
    }
    return NULL;
}

GIMS_Point *GIMS_MultiLineString::getPrevPoint(int index)
{
    for (int i = 0; i < this->size; i++) {
        if (index < this->list[i]->size)
            if (index - 1 > 0)
                return this->list[i]->list[index - 1];
            else
                return NULL;
        else
            index -= this->list[i]->size;
    }
    return NULL;
}

GIMS_Geometry *GIMS_MultiLineString::clipToBox(GIMS_BoundingBox *box)
{
    GIMS_MultiLineString *clipped = NULL;
    for (int i = 0; i < this->size; i++) {
        GIMS_MultiLineString *partial = (GIMS_MultiLineString *)this->list[i]->clipToBox(box);
        if (partial != NULL) {
            if (clipped == NULL) {
                clipped = new GIMS_MultiLineString(1);
                clipped->id = this->id;
            }
            clipped->merge(partial);
            delete partial;
        }
    }
    return clipped;
}

void GIMS_MultiLineString::merge(GIMS_MultiLineString *mls)
{
    int prevSize = this->size;
    this->size = this->size + mls->size;
    if (this->size > this->allocatedSize) {
        this->list = (GIMS_LineString **)realloc(this->list, this->size * sizeof(GIMS_LineString *));
        this->allocatedSize = this->size;
    }
    memcpy(this->list + prevSize, mls->list, mls->size * sizeof(GIMS_LineString *));
}

void GIMS_MultiLineString::append(GIMS_LineString *l)
{
    this->size += 1;
    if (this->size > this->allocatedSize) {
        this->list = (GIMS_LineString **)realloc(this->list, this->size * sizeof(GIMS_LineString *));
        this->allocatedSize = size;
    }
    this->list[this->size - 1] = l;
}

GIMS_MultiLineString::GIMS_MultiLineString(int size)
{
    this->type = MULTILINESTRING;
    this->size = 0;
    this->allocatedSize = size;
    this->list = (GIMS_LineString **)malloc(size * sizeof(GIMS_LineString *));
    this->id = 0;
}

GIMS_MultiLineString::GIMS_MultiLineString()
{
    this->type = MULTILINESTRING;
    this->size = this->allocatedSize = 0;
    this->list = NULL;
    this->id = 0;
}

GIMS_MultiLineString::~GIMS_MultiLineString()
{
    if (this->list != NULL)
        free(this->list);
}
