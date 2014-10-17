#include "PMQuadTree.hpp"

using namespace GIMSGEOMETRY;
using namespace PMQUADTREE;
using namespace std;

Quadrant quadrantList[] = {NW, NE, SE, SW};
char quadrantLabels[][3] = {"NW", "NE", "SE", "SW"};
double xmultiplier[] = {0.0, 0.5, 0.5, 0.0};
double ymultiplier[] = {0.0, 0.0, -0.5, -0.5};

/*
-->Polygonal Map QuadTree Node
*/
/* Returns true if the given geometry is a valid one for the calling node
   !Note! The bounding box geometry is not supported !Note!
   The behaviour is undefined in such a situation. */
bool Node::validateGeometry (GIMSGeometry *g) {

    if( g->type != MIXEDLIST )
        return true;

    GIMSGeometryList *geom = (GIMSGeometryList *)g;

    for ( list<GIMSGeometry *>::iterator it = geom->list->begin();
          it != geom->list->end();
          it++                                                          ) {

        if ( (*it)->type == POINT ) {
            return this->validateVertexSharing ( (GIMSPoint *)(*it), geom->list, it);
            
        }else if ( (*it)->type == EDGE ) {

            bool p1Inside = ((GIMSEdge *)(*it))->p1->isInsideBox( this->square ),
                 p2Inside = ((GIMSEdge *)(*it))->p2->isInsideBox( this->square );
            
            if ( p1Inside && p2Inside ){
                /*if the square has both edge endpoints contained*/
                return false;
                
            } else if ( p1Inside ) {
                return validateVertexSharing ( ((GIMSEdge *)(*it))->p1, geom->list, it);
                
            } else if ( p2Inside ) {
                return validateVertexSharing ( ((GIMSEdge *)(*it))->p2, geom->list, it);
            }
        }else{
            fprintf(stderr, "unsupported geometry was passed on to the node validation function." );
            exit(-1);
        }
    }
    
    return true;
}

/*check if all edges either contain Point P and don't have their other
vertex inside S, or don't have any of the two vertexes inside S*/
bool Node::validateVertexSharing ( GIMSPoint *pt, 
                                   list<GIMSGeometry *> *geom, 
                                   list<GIMSGeometry *>::iterator it ) {
                             
    for (it++; it != geom->end(); it++ ) {

        if ( (*it)->type == EDGE ) {

            if ( ((GIMSEdge *)(*it))->p1 == pt ) {
                if ( ((GIMSEdge *)(*it))->p2->isInsideBox(this->square) ) {
                    return false;
                }
                
            } else if ( ((GIMSEdge *)(*it))->p2 == pt ) {
                if ( ((GIMSEdge *)(*it))->p1->isInsideBox(this->square) ) {
                    return false;
                }
                
            } else if ( ((GIMSEdge *)(*it))->p1->isInsideBox(this->square) ||
                        ((GIMSEdge *)(*it))->p2->isInsideBox(this->square) ) {
                return false;
            }
            
        } else if ( (*it)->type == POINT ) {
            if ( ((GIMSPoint *)(*it)) != pt &&
                 ((GIMSPoint *)(*it))->isInsideBox(this->square) ) {
                return false;
            }
        }
    }
    
    return true;
}

/*creates four new sons (one for each quadrant) in the calling node*/
void Node::split(){
    double xlen = this->square->xlength(),
           ylen = this->square->ylength();
    
    for (Quadrant q : quadrantList) {
        /*create and link corresponding square*/
        GIMSBoundingBox *square = 
            new GIMSBoundingBox(
                new GIMSPoint( this->square->minx() + xlen * xmultiplier[q],
                               this->square->miny() + ylen * ymultiplier[q]
                             ),
                new GIMSPoint( this->square->minx() + xlen * (xmultiplier[q] + 0.5),
                               this->square->miny() + ylen * (ymultiplier[q] + 0.5)
                             )
                );
        /*create and append a new node*/
        this->sons[q] = new Node(square);
    }
    
    this->type = GRAY;
}

Node::Node(){
    this->type = WHITE;
    this->square = NULL;
    this->dictionary = new list<GIMSGeometry *>;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
}

Node::Node( GIMSBoundingBox *square ){
    this->type = WHITE;
    this->square = square;
    this->dictionary = new list<GIMSGeometry *>;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
}

Node::~Node(){
    delete this->square;
    delete this->dictionary;
}
/*
Polygonal Map QuadTree Node<--
*/




PMQuadTree::PMQuadTree (GIMSBoundingBox *domain) {

}

PMQuadTree::~PMQuadTree () {

}



/*Functions that take care of the construction and maintenance of the structure*/
void PMQuadTree::build  (GIMSGeometry *geom){

}



void PMQuadTree::insert (GIMSGeometry *geom){

}

void PMQuadTree::remove (GIMSGeometry *geom){

}

void *PMQuadTree::search (GIMSGeometry *geom){
    return NULL;
}



/*Follow the operations between the data structure and a given geometry*/
RelStatus PMQuadTree::intersects_g  ( GIMSGeometry *result, GIMSGeometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::meets_g       ( GIMSGeometry *result, GIMSGeometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::contains_g    ( GIMSGeometry *result, GIMSGeometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::isContained_g ( GIMSGeometry *result, GIMSGeometry *geom){
    return UNDECIDED_RELATIONSHIP;
}



/*Retrieve all geometry elements that are partially or totally contained
  in a given bounding box*/
RelStatus PMQuadTree::isBoundedBy ( GIMSGeometry *result, GIMSBoundingBox *box){
    return UNDECIDED_RELATIONSHIP;
}
