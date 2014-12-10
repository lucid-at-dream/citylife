#include "PMQuadTree.hpp"

using namespace GIMSGEOMETRY;
using namespace PMQUADTREE;
using namespace std;

Quadrant quadrantList[4] =  {NW, NE, SE, SW};
char quadrantLabels[4][3] =  {"NW", "NE", "SE", "SW"};
double xmultiplier[4] =  {0.0, 0.5, 0.5, 0.0};
double ymultiplier[4] =  {0.5, 0.5, 0.0, 0.0};
int depth = 0;

bool renderEdge = false;
list<GIMSGeometry *> *renderQueue = new list<GIMSGeometry *>();
list<GIMSGeometry *> *redRenderQueue = new list<GIMSGeometry *>();

/*
-->Polygonal Map QuadTree Node
*/

/*Returns all nodes of the Polygonal Map QuadTree that intersect the border of
  the geometry passed by parameter. Note: This means that this function does not 
  return nodes strictly contained inside polygons.*/
void *Node::search (GIMSGeometry *geom){

    GIMSGeometry *clipped = geom->clipToBox(this->square);
    if( clipped == NULL )
        return NULL;

    if ( this->type != GRAY ) {
        list<Node *> *l = new list<Node *>;
        l->push_back(this);
        delete clipped;
        return l;
    }else{
        list<Node *> *retlist = new list<Node *>;
        for( Quadrant q : quadrantList ) {
            list<Node *> *l = (list<Node *> *)sons[q]->search(clipped);
            if( l != NULL ){
                retlist->insert( retlist->end(), l->begin(), l->end() );
                delete l;
            }
        }
        delete clipped;
        return retlist;
    }
}

/*Returns a neighboor node in the north direction. Chooses the neighboor node 
  that intersects the vertical line defined by x = <param:x>. If two northen 
  neighboors are intersected, returns one of the two without criterion.*/
Node *Node::goNorth( double x ){

    if( this->father == NULL )
        return NULL;

    //we simply create a point out of the node, a little upper and search for it.
    GIMSPoint pt = GIMSPoint(x, this->square->upperRight->y);
    pt.y += ERR_MARGIN * 4;

    //we first get the first node above this that contains the created point.
    Node *node;
    Node *cNode = this->father;
    while( !pt.isInsideBox(cNode->square) ){
        if(cNode->father != NULL){
            //since the lower node didn't contain pt, we go higher
            cNode = cNode->father;
        }else{
            //we can't go higher, which means that the point is out of the map
            return NULL;
        }
    }

    //we then search for the point in the found node to get the containing leaf node.
    //even tough there's no guarantee that only one node will be returned, there's a guarantee
    //that all the retrieved nodes intersect the vertical line that goes through x
    node = ((list<Node *> *)(cNode->search( &pt )))->front();

    return node;
}

/*if node has reference to a geometry labeled with id, returns the geometry's 
  reference, else returns NULL*/
GIMSGeometry *Node::hasReferenceTo( unsigned long int id ){

    if( this->dictionary == NULL )
        return NULL;

    if( this->dictionary->size() == 0 )
        return NULL;

    for( list<GIMSGeometry *>::iterator it = this->dictionary->begin();
         it != this->dictionary->end(); it++ ){
        if( (*it)->id == id ){
            return *it;
        }
    }
    return NULL;
}

/*Returns true if a polygon "pol" that intersects the node contains point "pt".*/
bool Node::polygonContainsPoint(GIMSPolygon *pol, GIMSPoint *pt){
    /*Explanation:
      According to the paper where pmqtrees are proposed, the procedure to label
      a point is as follows:
        >from the node containing the point go straight in one direction until a
      node containing edges is found.
        >from the edges contained in such a node choose the one that is closest
      to query point.
        >if such an edge does not share a vertex that is inside the node, then
      check to which side of the edge the query point lies and report accordingly.
        >otherwise, from the edges sharing the vertex inside the node, choose
        the one that forms the smallest angle with the hipotetical edge that goes 
        from the query point to the vertex, check to which side of the chosen edge
        the point lies and report accordingly.*/

    /*find the first node in the north direction than intersects polygon pol*/
    Node *n = this;
    GIMSGeometry *first;

    while( (first = n->hasReferenceTo( pol->id )) == NULL ){
        n = n->goNorth( pt->x );
        if( n == NULL )
            return false;
    }

    //set up the query point
    GIMSPoint *qp = n->square->lowerLeft->clone();
    qp->x = pt->x;

    //The first portion of the polygon "pol" that we found going in the north direction
    GIMSPolygon *p = (GIMSPolygon *)first;

    /*from all the line segments that belong to polygon pol and that intersect 
      the node we just found, find out which one is closest to point pt.*/
    
    //keep track whether the closes edge is from an internal ring or the external
    //ring, as it will affect the side the point should be on to be contained.
    bool isEdgeFromExtRing = false;
    double minDist = 1e100;
    double tmp;
    GIMSEdge *closest = NULL;
    //Keep track of the adjacent edges in order to account for vertex sharing within the node.
    GIMSEdge *closestPrev = NULL,
             *closestNext = NULL;
    GIMSEdge *prev = NULL;

    //iterate over the edges from the external ring
    for( list<GIMSGeometry *>::iterator it = p->externalRing->list->begin();
         it != p->externalRing->list->end(); it++ ){
        if( (tmp = distToSegmentSquared( qp, (GIMSEdge *)(*it) )) < minDist ){
            minDist = tmp;
            closest = (GIMSEdge *)(*it);
            closestPrev = prev;
            list<GIMSGeometry *>::iterator aux = it;
            aux++;
            if( aux != p->externalRing->list->end() )
                closestNext = (GIMSEdge *)(*aux);
            else
                closestNext = NULL;
            isEdgeFromExtRing = true;
        }
        prev = (GIMSEdge *)(*it);
    }

    //iterate over the edges from the internal rings
    for( list<GIMSGeometry *>::iterator int_ring = p->internalRings->list->begin();
         int_ring != p->internalRings->list->end(); int_ring++ ){
        prev = NULL;
        for( list<GIMSGeometry *>::iterator it = ((GIMSGeometryList *)(*int_ring))->list->begin();
             it != ((GIMSGeometryList *)(*int_ring))->list->end(); it++ ){
            if( (tmp = distToSegmentSquared( qp, (GIMSEdge *)(*it) )) < minDist ){
                minDist = tmp;
                closest = (GIMSEdge *)(*it);
                closestPrev = prev;
                list<GIMSGeometry *>::iterator aux = it;
                aux++;
                if( aux != ((GIMSGeometryList *)(*int_ring))->list->end() )
                    closestNext = (GIMSEdge *)(*aux);
                else
                    closestNext = NULL;
                isEdgeFromExtRing = false;
            }
        }
    }

    /*if one of the closest line segment endpoints lies within the square that
      bounds node n, we must get the 2nd edge that shares the same endpoint.
      We can call the shared enpoint pt2.
      Given this convention, we must now check which of the two line segments 
      forms the smallest angle with the linesegment pt---pt2. We then check to 
      with side of that line pt lies.*/
    bool p1inside = closest->p1->isInsideBox(n->square),
         p2inside = closest->p2->isInsideBox(n->square);

    if( p1inside || p2inside ){
        GIMSPoint *pt2 = p1inside ? closest->p1 : closest->p2;
        GIMSPoint *unshared1 = p1inside ? closest->p2 : closest->p1;

        GIMSEdge *e1 = closest, *e2;
        if( closestPrev != NULL && (closestPrev->p1->equals(pt2) || closestPrev->p2->equals(pt2)) ){
            e2 = closestPrev;
        }else{
            if( closestNext != NULL && (closestNext->p1->equals(pt2) || closestNext->p2->equals(pt2)) ){
                e2 = closestNext;
            }else{
                //if this is reached, it means that even though an endpoint of the
                //closest edge lied inside the node, the edge didn't have adjacent
                //edges, which means the ring is not closed, which is not possible.
                fprintf(stderr, "@polygonContainsPoint - !Bug Alert! - open ring?\n");
                fprintf(stderr, "fatal, cannot proceed with this function.\n");
                return false;
            }
        }
        GIMSPoint *unshared2 = e2->p1->equals(pt2) ? e2->p2 : e2->p1;

        //compute and compare angles
        double angle1 = angle3p(unshared1, pt2, qp),
               angle2 = angle3p(unshared2, pt2, qp);

        closest = angle1 < angle2 ? e1 : e2;
    }

    /*check which side of closest pt lies in*/
    if( isEdgeFromExtRing ){
        if( qp->sideOf(closest) == RIGHT )
            return true;
        else
            return false;
    }else{
        if( qp->sideOf(closest) == LEFT )
            return true;
        else
            return false;
    }
}

/*Returns all nodes that intersect a given poligon "pol", including the 
  polygon's interior. This means that this function returns all tree nodes that
  are strictly contained in the polygon.*/
void *Node::searchInterior (GIMSPolygon *pol){

    if ( this->type != GRAY ) { //if the node is a leaf node
        list<Node *> *l = new list<Node *>;
        l->push_back(this);
        return l;
    }else{
        list<Node *> *retlist = new list<Node *>;
        
        GIMSPolygon *clipped;
        for( Quadrant q : quadrantList ) {
            //check for intersections between pol and the node
            clipped = (GIMSPolygon *)(pol->clipToBox(sons[q]->square));

            bool contained = false;
            if( clipped == NULL ){
                //if there are no intersections, then either the node is strictly
                //contained in the polygon interior or exterior
                GIMSPoint p = GIMSPoint( sons[q]->square->upperRight->x - 4 * ERR_MARGIN,
                                         sons[q]->square->upperRight->y                 );
                contained = sons[q]->polygonContainsPoint( pol, &p );
            }

            if( contained ){
                //if the node is contained, we can report it free of charge
                retlist->push_back(sons[q]);
            }else if( clipped != NULL ){
                //if the node is intersected we call recursively to its sons.
                list<Node *> *l = (list<Node *> *)sons[q]->searchInterior(clipped);
                if( l != NULL ){
                    retlist->insert( retlist->end(), l->begin(), l->end() );
                    delete l;
                }
            }
        }
        return retlist;
    }
}

/*Inserts geometry "geom" in the tree*/
void Node::insert ( GIMSGeometry *geom ) {

    depth++;
    if( depth > 100 ){
        fprintf(stderr, "max depth reached (100), not inserting. Look out for bugs...\n");
        depth--;
        return;
    }

    GIMSGeometry *clipped = (GIMSGeometry *)(geom->clipToBox ( this->square ));

    if (clipped == NULL) { //geometry to insert does not intersect this node
        depth--;
        return;
    }

    if ( this->type != GRAY ) { //node type is only gray when it's not a leaf
                                //we're therefore accessing leaf nodes in this block

        
        if (this->dictionary != NULL) { //merge clipped with the node's dictionary
            if( clipped->type == MIXEDLIST ){
                ((GIMSGeometryList *)(clipped))->list->insert (
                        ((GIMSGeometryList *)(clipped))->list->end(),
                        this->dictionary->begin(),
                        this->dictionary->end() );
                delete this->dictionary;
                this->dictionary = NULL;
            }else{
                this->dictionary->push_back(clipped);
                clipped = new GIMSGeometryList();
                ((GIMSGeometryList *)(clipped))->list = this->dictionary;
                this->dictionary = NULL;
            }
        }

        if ( this->validateGeometry ( clipped ) ) {
            //if the geometry is a valid node geometry we insert it into the node
            this->type = BLACK;
            this->dictionary = ((GIMSGeometryList *)(clipped))->list;
            depth--;
            return;
        } else {
            this->split();
        }
    }

    for (Quadrant q : quadrantList) { //recurse
        this->sons[q]->insert ( clipped );
    }
    depth--;
    delete clipped;
}

/* Returns true if the given geometry is a valid one for the calling node
   !Note! The bounding box geometry is not supported.
   The behaviour is undefined in such a situation.!Note! */
bool Node::validateGeometry (GIMSGeometry *g) {

    //printf("datatype: %d\n", g->type);
    if( g->type != MIXEDLIST ){
        return true;
    }

    GIMSPoint *contained = NULL;

    GIMSGeometryList *geom = (GIMSGeometryList *)g;
    for ( list<GIMSGeometry *>::iterator it = geom->list->begin(); it != geom->list->end(); it++ ) {

        if ( (*it)->type == POINT ) {
            if(contained == NULL)
                contained = (GIMSPoint *)(*it);
            else{
                if( !((GIMSPoint *)(*it))->equals(contained) )
                    return false;
            }

        }else if ( (*it)->type == EDGE ) {

            bool p1Inside = ((GIMSEdge *)(*it))->p1->isInsideBox( this->square ),
                 p2Inside = ((GIMSEdge *)(*it))->p2->isInsideBox( this->square );

                if( p1Inside && contained != NULL && !((GIMSEdge *)(*it))->p1->equals(contained) )
                    return false;
                if( p2Inside && contained != NULL && !((GIMSEdge *)(*it))->p2->equals(contained) )
                    return false;
                if ( p1Inside && p2Inside )
                    return false;

                if( contained == NULL ){
                    if(p1Inside)
                        contained = ((GIMSEdge *)(*it))->p1;
                    else if(p2Inside)
                        contained = ((GIMSEdge *)(*it))->p2;
                }

        }else if( (*it)->type == POLYGON ){
            GIMSPolygon *p = (GIMSPolygon *)(*it);

            //validate the external ring
            for ( list<GIMSGeometry *>::iterator ext = p->externalRing->list->begin(); ext != p->externalRing->list->end(); ext++ ){
                bool p1Inside = ((GIMSEdge *)(*ext))->p1->isInsideBox( this->square ),
                     p2Inside = ((GIMSEdge *)(*ext))->p2->isInsideBox( this->square );
                if( (contained != NULL && p1Inside && !((GIMSEdge *)(*ext))->p1->equals(contained)) ||
                    (contained != NULL && p2Inside && !((GIMSEdge *)(*ext))->p2->equals(contained)) ||
                    (p1Inside && p2Inside) ){
                    return false;
                }else if(contained == NULL){
                    if(p1Inside)
                        contained = ((GIMSEdge *)(*ext))->p1;
                    if(p2Inside)
                        contained = ((GIMSEdge *)(*ext))->p2;
                }
            }

            //iterate over the internal rings
            for ( list<GIMSGeometry *>::iterator ir = p->internalRings->list->begin(); ir != p->internalRings->list->end(); ir++ ){
                //and validate each of those internal rings
                for( list<GIMSGeometry *>::iterator edge = ((GIMSGeometryList *)(*ir))->list->begin(); edge != ((GIMSGeometryList *)(*ir))->list->end(); edge++ ){
                    bool p1Inside = ((GIMSEdge *)(*edge))->p1->isInsideBox( this->square ),
                         p2Inside = ((GIMSEdge *)(*edge))->p2->isInsideBox( this->square );
                    if( (contained != NULL && p1Inside && !((GIMSEdge *)(*edge))->p1->equals(contained)) ||
                        (contained != NULL && p2Inside && !((GIMSEdge *)(*edge))->p2->equals(contained)) ||
                        (p1Inside && p2Inside) )
                        return false;
                    else if(contained == NULL){
                        if(p1Inside)
                            contained = ((GIMSEdge *)(*edge))->p1;
                        if(p2Inside)
                            contained = ((GIMSEdge *)(*edge))->p2;
                    }
                }
            }
        }else{
            fprintf(stderr, "unsupported geometry was passed on to the node validation function." );
            exit(-1);
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
        this->sons[q]->father = this;
    }
    
    this->type = GRAY;
}

Node::Node(){
    this->type = WHITE;
    this->square = NULL;
    this->dictionary = new list<GIMSGeometry *>;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
    this->father = NULL;
}

Node::Node( GIMSBoundingBox *square ){
    this->type = WHITE;
    this->square = square;
    this->dictionary = new list<GIMSGeometry *>;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
    this->father = NULL;
}

Node::~Node(){
    delete this->square;
    delete this->dictionary;
}
/*
Polygonal Map QuadTree Node<--
*/




PMQuadTree::PMQuadTree (GIMSBoundingBox *domain) {
    this->root = new Node( domain );
    this->query = NULL;
}

PMQuadTree::~PMQuadTree () {}



/*Functions that take care of the construction and maintenance of the structure*/
void PMQuadTree::build  (GIMSGeometry *geom){}

void PMQuadTree::insert ( GIMSGeometry *geom ) {
    this->root->insert(geom);
}

void PMQuadTree::remove (GIMSGeometry *geom){

}

/*return all leaf nodes that intersect geom*/
void *PMQuadTree::search (GIMSGeometry *geom){
    return this->root->search(geom);
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










/* Functions for debug renderization module */
void PMQuadTree::debugRender(Cairo::RefPtr<Cairo::Context> cr){

    renderer->setScale( 400.0/this->root->square->xlength(),
                        -400.0/this->root->square->ylength() );
    renderer->setTranslation( -this->root->square->lowerLeft->x,
                              -this->root->square->upperRight->y );
    this->renderTree ( cr, this->root );
    printf("rendered the tree\n");

    if(query != NULL){
        list<Node *> *results;
        if( query->type == POLYGON )
            results = (list<Node *> *)(this->root->searchInterior( (GIMSPolygon *)(this->query) ));
        else
            results = (list<Node *> *)(this->root->search( this->query ));
        cr->set_source_rgba(0.0, 0.19, 0.69, 0.2);
        for( list<Node *>::iterator i = results->begin(); i!= results->end(); i++ ){
            renderer->renderFilledBBox( cr, (*i)->square );
            cr->fill();
        }
    }else{
        printf("null query\n");
    }

    cr->set_source_rgba(0.19, 0.73, 0.12, 0.5);
    for(list<GIMSGeometry *>::iterator it = renderQueue->begin(); it != renderQueue->end(); it++){
        renderer->renderGeometry(cr, *it);
    }

    cr->set_source_rgba(0.73, 0.19, 0.03, 0.5);
    for(list<GIMSGeometry *>::iterator it = redRenderQueue->begin(); it != redRenderQueue->end(); it++){
        renderer->renderGeometry(cr, *it);
    }

}

/*Recursively render the tree nodes*/
void PMQuadTree::renderTree (Cairo::RefPtr<Cairo::Context> cr, Node *n) {
    /*if it is a leaf node*/
    if (n->type != GRAY) {
        this->renderLeafNode (cr, n);
    } else {
        for (Quadrant q : quadrantList) {
            this->renderTree (cr, n->sons[q]);
        }
    }
}

/*Render a leaf node and contained geometries*/
void PMQuadTree::renderLeafNode (Cairo::RefPtr<Cairo::Context> cr, Node *n) {

    renderer->renderGeometry(cr, n->square);

    if( n->dictionary == NULL )
        return;

    if (n->type == BLACK) { //the WHITE type stands for empty node, thus we ignore it.
        for ( list<GIMSGeometry *>::iterator it = n->dictionary->begin();
              it != n->dictionary->end(); it++ ) {

            if( (*it)->renderCount >= renderer->renderCount )
                continue;

            if ( (*it)->type == EDGE ) {
                //GIMSEdge *trimmed =  ((GIMSEdge*)(*it))->trimToBBox(n->square);
                //renderer->renderGeometry( cr, trimmed );
                //delete trimmed;
                renderer->renderGeometry( cr, *it );
                (*it)->renderCount++;
            }else if( (*it)->type == POINT ) {
                renderer->renderGeometry( cr, (GIMSPoint*)(*it) );
                (*it)->renderCount++;
            }else if( (*it)->type == POLYGON ) {
                (*it)->renderCount++;
                renderer->renderGeometry( cr, ((GIMSPolygon *)(*it))->externalRing );
                cr->set_source_rgb(0.69, 0.19, 0.0);
                renderer->renderGeometry( cr, ((GIMSPolygon *)(*it))->internalRings );
                cr->set_source_rgb(0.0, 0.0, 0.0);
            }
        }
    }
}

void PMQuadTree::onClick( double x, double y){
    printf("begin click event\n");

    /*
    GIMSPolygon *pol= (GIMSPolygon *)(((GIMSGeometryList *)(this->query))->list->front());
    GIMSPoint *pt = new GIMSPoint(x,y);
    renderEdge = true;
    Node *n = ((list<Node *> *)(this->root->search(pt)))->front();
    n->polygonContainsPoint(pol, pt);
    renderEdge = false;
    */

    printf("finished click event\n");
}
