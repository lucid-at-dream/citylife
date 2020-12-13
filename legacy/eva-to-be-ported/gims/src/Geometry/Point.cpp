#include "Geometry.hpp"

/* Returns true if the point lies inside the given bounding box */
int GIMS_Point::getPointCount()
{
        return 1;
}

void GIMS_Point::deleteClipped()
{
}

void GIMS_Point::deepDelete()
{
        delete this;
}

double GIMS_Point::distance(GIMS_Point *p)
{
        return sqrt((this->x - p->x) * (this->x - p->x) + (this->y - p->y) * (this->y - p->y));
}

double GIMS_Point::distanceSquared(GIMS_Point *p)
{
        return (this->x - p->x) * (this->x - p->x) + (this->y - p->y) * (this->y - p->y);
}

GIMS_Point GIMS_Point::getClosestPoint(GIMS_LineSegment *e)
{
        //Length of the line segment squared
        double lineSegLenSquared =
                (e->p1->x - e->p2->x) * (e->p1->x - e->p2->x) + (e->p1->y - e->p2->y) * (e->p1->y - e->p2->y);

        if (lineSegLenSquared == 0)
                //if true then the line segment is actually a point.
                return *(e->p1);

        //dot product of (p - e->p1, e->p2 - e->p1)
        double dotProduct = (this->x - e->p1->x) * (e->p2->x - e->p1->x) + (this->y - e->p1->y) * (e->p2->y - e->p1->y);

        /*If we define the line that extends edge "e" as l(t) = e->p1 + t * (e->p2 - e->p1)
      The closest point of "p" that lies in line "l", is a point "cp" such that
      the line p(t) = p + t *(cp - p) forms a 90º degree angle with line "l".
      In order to find the value of t in l(t) for point cp, we have 
      t = [(p - e->p1).(e->p2 - e->p1)] / |e->p2 - e->p1|^2*/
        double t = dotProduct / lineSegLenSquared;
        if (t < 0) {
                //if t < 0 then cp lies beyond e->p1, and therefore e->p1 is the closest point
                return *(e->p1);
        }
        if (t > 1) {
                //if t < 0 then cp lies beyond e->p2, and therefore e->p2 is the closest point
                return *(e->p2);
        }

        //if t is in [0,1], then we must calculate point cp (nearest_pt)
        double nearest_pt_x = e->p1->x + t * (e->p2->x - e->p1->x), nearest_pt_y = e->p1->y + t * (e->p2->y - e->p1->y);
        return GIMS_Point(nearest_pt_x, nearest_pt_y);
}

GIMS_Point *GIMS_Point::clone()
{
        GIMS_Point *fresh = new GIMS_Point(this->x, this->y);
        fresh->id = this->id;
        return fresh;
}

string GIMS_Point::toWkt()
{
        char buff[100];
        sprintf(buff, "POINT(%lf %lf)", this->x, this->y);
        return buff;
}

/*Returns true if the point lies inside the parameter bounding box*/
bool GIMS_Point::isInsideBox(GIMS_BoundingBox *box)
{
        if (this->x <= box->upperRight->x + ERR_MARGIN && //xmax
            this->x >= box->lowerLeft->x - ERR_MARGIN && //xmin
            this->y <= box->upperRight->y + ERR_MARGIN && //ymax
            this->y >= box->lowerLeft->y - ERR_MARGIN //ymin
        ) {
                return true;
        }
        return false;
}

/*Returns true if the point lies inside the parameter bounding box*/
bool GIMS_Point::isContainedInBox(GIMS_BoundingBox *box)
{
        if (this->x < box->upperRight->x - ERR_MARGIN && //xmax
            this->x > box->lowerLeft->x + ERR_MARGIN && //xmin
            this->y < box->upperRight->y - ERR_MARGIN && //ymax
            this->y > box->lowerLeft->y + ERR_MARGIN //ymin
        ) {
                return true;
        }
        return false;
}

/* Returns to which side of the vector defined by (edge->p1, edge->p2) the point
   lies. The sides are defined in the enum GIMS_Side */
GIMS_Side GIMS_Point::sideOf(GIMS_LineSegment *edge)
{
        double s = (edge->p2->x - edge->p1->x) * (this->y - edge->p1->y) -
                   (edge->p2->y - edge->p1->y) * (this->x - edge->p1->x);
        return s < 0 ? RIGHT : (s > 0 ? LEFT : ALIGNED);
}

/*if the point lies in the interior of the bounding box, returns the point, else null*/
GIMS_Geometry *GIMS_Point::clipToBox(GIMS_BoundingBox *box)
{
        if (this->isInsideBox(box))
                return this;
        else
                return NULL;
}

/*returns true if the coordinates of both points are the same. false otherwise.*/
bool GIMS_Point::equals(GIMS_Point *cmp)
{
        if (this->x <= cmp->x + ERR_MARGIN && this->x >= cmp->x - ERR_MARGIN && this->y <= cmp->y + ERR_MARGIN &&
            this->y >= cmp->y - ERR_MARGIN) {
                return true;
        }
        return false;
}

GIMS_Point::GIMS_Point()
{
        this->id = 0;
        this->type = POINT;
        this->extractedFromDatabase = false;
}

GIMS_Point::GIMS_Point(double x, double y, bool efd)
{
        this->id = 0;
        this->x = x;
        this->y = y;
        this->type = POINT;
        this->extractedFromDatabase = efd;
}

GIMS_Point::~GIMS_Point()
{
}

/*Multi point implementation below*/
int GIMS_MultiPoint::getPointCount()
{
        return size;
}

void GIMS_MultiPoint::deepDelete()
{
        for (int i = 0; i < this->size; i++) {
                delete list[i];
        }
        delete this;
}

void GIMS_MultiPoint::deleteClipped()
{
        delete this;
}

void GIMS_MultiPoint::append(GIMS_Point *pt)
{
        this->size += 1;
        if (this->size > this->allocatedSize) {
                this->list = (GIMS_Point **)realloc(this->list, this->size * sizeof(GIMS_Point *));
                this->allocatedSize = this->size;
        }
        this->list[this->size - 1] = pt;
}

string GIMS_MultiPoint::toWkt()
{
        string wkt = string("MULTIPOINT(");
        char buff[100];
        for (int i = 0; i < this->size; i++) {
                sprintf(buff, "%lf %lf", this->list[i]->x, this->list[i]->y);
                wkt += string(buff) + (i < this->size - 1 ? "," : ")");
        }
        return wkt;
}

GIMS_MultiPoint *GIMS_MultiPoint::clone()
{
        GIMS_MultiPoint *fresh = new GIMS_MultiPoint(this->allocatedSize);
        memcpy(fresh->list, this->list, this->size * sizeof(GIMS_Point *));
        fresh->size = this->size;
        fresh->id = this->id;
        return fresh;
}

GIMS_Geometry *GIMS_MultiPoint::clipToBox(GIMS_BoundingBox *box)
{
        GIMS_MultiPoint *clipped = NULL;
        for (int i = 0; i < this->size; i++) {
                if (this->list[i]->isInsideBox(box)) {
                        if (clipped == NULL) {
                                clipped = new GIMS_MultiPoint(1);
                                clipped->id = this->id;
                        }
                        clipped->append(this->list[i]);
                }
        }
        return clipped;
}

GIMS_MultiPoint::GIMS_MultiPoint(int size)
{
        this->type = MULTIPOINT;
        this->id = 0;
        this->size = 0;
        this->allocatedSize = size;
        this->list = (GIMS_Point **)malloc(size * sizeof(GIMS_Point *));
}

GIMS_MultiPoint::GIMS_MultiPoint()
{
        this->type = MULTIPOINT;
        this->id = 0;
        this->size = this->allocatedSize = 0;
        this->list = NULL;
}

GIMS_MultiPoint::~GIMS_MultiPoint()
{
        free(this->list);
}
