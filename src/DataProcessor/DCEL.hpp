#ifndef _DCEL_HPP_
#define _DCEL_HPP_

#include "Geometry.hpp"
#include <set>
#include <math.h>

class vertex;
class face;
class halfedge;

class vertex{
  public:
    GIMS_Point *pt;
    list<halfedge *> incidentEdges;
    int data;

    vertex();

    static int compare(const vertex &a, const vertex &b){
        long long int ax = a.pt->x * 100000000.0,
                      ay = a.pt->y * 100000000.0,
                      bx = b.pt->x * 100000000.0,
                      by = b.pt->y * 100000000.0;

        ax /= 100; ay /= 100; bx /= 100; by /= 100;

        if( ax < bx)
            return 1;
        else if(ax > bx)
            return -1;
        else if( ay < by)
            return 1;
        else if(ay > by)
            return -1;
        return 0;
    }
};
struct vertex_cmp {
    bool operator() (const vertex *a, const vertex *b) const{
        return vertex::compare(*a, *b) > 0;
    }
};

class halfedge{
  public:
    halfedge *prev; /*previous half edge*/
    halfedge *next; /*next half edge*/
    halfedge *twin; /*twin half edge*/

    vertex   *tail; /*tail vertex*/
    face     *left; /*face to the left of the edge (bounded by this halfedge)*/

    int count;
    int data;

    halfedge();

    static int compare(const halfedge &a, const halfedge &b){
        int first = vertex::compare(*(a.tail), *(b.tail));
        if( first != 0 )
            return first;
        int second = vertex::compare(*(a.twin->tail), *(b.twin->tail));
        return second;
    }
};
struct halfedge_cmp {
    bool operator() (const halfedge *a, const halfedge *b) const{
        return halfedge::compare(*a, *b) > 0;
    }
};

class face{
  public:
    face();

    halfedge *boundary;
    int data;
};

typedef set<vertex *, vertex_cmp> vertexlist;
typedef set<halfedge *, halfedge_cmp> halfedgelist;
typedef list<face *> facelist;

class DCEL{
  public:

    vertexlist   vertexes;
    halfedgelist halfedges;
    facelist     faces;

    void addVertex(vertex *item);
    vertex *findVertex(GIMS_Point *p);
    vertex *findVertex(vertex *v);

    void addHalfedge(halfedge *item);
    halfedge *findHalfedge(halfedge *h);

    void addFace(face *item);
    void calculateFaces();
};

bool sort_clockwise_cmp_(const halfedge *a, const halfedge *b);

#endif