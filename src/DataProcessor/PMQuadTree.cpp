#include "PMQuadTree.hpp"

using namespace GIMS_GEOMETRY;
using namespace PMQUADTREE;
using namespace std;

Quadrant quadrantList[4] =  {NW, NE, SE, SW};
char quadrantLabels[4][3] =  {"NW", "NE", "SE", "SW"};
double xmultiplier[4] =  {0.0, 0.5, 0.5, 0.0};
double ymultiplier[4] =  {0.5, 0.5, 0.0, 0.0};
int depth = 0;

bool renderEdge = false;
list<GIMS_Geometry *> *renderQueue = new list<GIMS_Geometry *>();
list<GIMS_Geometry *> *redRenderQueue = new list<GIMS_Geometry *>();

/*
-->Polygonal Map QuadTree Node
*/

/*Returns all nodes of the Polygonal Map QuadTree that intersect the border of
  the geometry passed by parameter. Note: This means that this function does not 
  return nodes strictly contained inside polygons.*/
void *Node::search (GIMS_Geometry *geom){

    if ( this->type != GRAY ) {
        list<Node *> *l = new list<Node *>;
        l->push_back(this);
        return l;
    
    }else{
        GIMS_Geometry *clipped = geom->clipToBox(this->square);
        if( clipped == NULL )
            return NULL;

        list<Node *> *retlist = new list<Node *>;
        for( Quadrant q : quadrantList ) {
            list<Node *> *l = (list<Node *> *)sons[q]->search(clipped);
            if( l != NULL ){
                retlist->insert( retlist->end(), l->begin(), l->end() );
                delete l;
            }
        }
        //we check if it's a point before deleting, because clipping a point returns a pointer
        //to the point itself and not to another geometry.
        if( clipped->type != POINT )
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
    GIMS_Point pt = GIMS_Point(x, this->square->upperRight->y);
    pt.y += ERR_MARGIN * 2;

    //we first get the first node above this that contains the created point.
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
    return ((list<Node *> *)(cNode->search( &pt )))->front();
}

/*if node has reference to a geometry labeled with id, returns the geometry's 
  reference, else returns NULL*/
GIMS_Geometry *Node::hasReferenceTo( long id ){
    if( this->dictionary == NULL )
        return NULL;

    for(list<GIMS_Geometry *>::iterator it = this->dictionary->begin();
        it != this->dictionary->end(); it++){
        if( (*it)->id == id )
            return *it;
    }
    return NULL;
}

/*Returns true if a polygon "pol" that intersects the node contains point "pt".*/
bool Node::polygonContainsPoint(GIMS_Polygon *pol, GIMS_Point *pt){
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
    GIMS_Geometry *first;

    while( (first = n->hasReferenceTo( pol->id )) == NULL ){
        n = n->goNorth( pt->x );
        if( n == NULL )
            return false;
    }

    //set up the query point
    GIMS_Point qp = GIMS_Point(pt->x, n->square->lowerLeft->y);

    //The first portion of the polygon "pol" that we found going in the north direction
    GIMS_Polygon *p = (GIMS_Polygon *)first;

    /*from all the line segments that belong to polygon pol and that intersect 
      the node we just found, find out which one is closest to point pt.*/
    
    //keep track whether the closest edge is from an internal ring or the external
    //ring, as it will affect the side the point should be on to be contained.
    bool isEdgeFromExtRing = false;
    double minDist = 1e100;
    double tmp;
    int min_i, min_j;
    GIMS_LineSegment closest;

    //iterate over the edges from the external ring
    for(int i=0; i<p->externalRing->size; i++){
        for(int j=0; j<p->externalRing->list[i]->size-1; j++){
            GIMS_LineSegment curr = p->externalRing->list[i]->getLineSegment(j);
            if( (tmp = distToSegmentSquared(&qp, &curr)) < minDist ){
                minDist = tmp; min_i = i; min_j = j; closest = curr;
                isEdgeFromExtRing = true;
            }
        }
    }

    //iterate over the edges from the internal ring
    for(int i=0; i<p->internalRings->size; i++){
        for(int j=0; j<p->internalRings->list[i]->size-1; j++){
            GIMS_LineSegment curr = p->internalRings->list[i]->getLineSegment(j);
            if((tmp = distToSegmentSquared(&qp, &curr)) < minDist){
                minDist = tmp; min_i = i; min_j = j; closest = curr;
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

    GIMS_MultiLineString *src = isEdgeFromExtRing ? p->externalRing : p->internalRings;

    bool p1inside = closest.p1->isInsideBox(n->square),
         p2inside = closest.p2->isInsideBox(n->square);

    if( p1inside || p2inside ){
        GIMS_Point *shpoint = p1inside ? closest.p1 : closest.p2;
        GIMS_Point *unshared1 = p1inside ? closest.p2 : closest.p1;
        GIMS_Point *unshared2;
        GIMS_LineSegment other;

        //find the edge that shares the endpoint with current "closest"
        bool found = false;
        for(int i=0; i<src->size && !found; i++){
            for(int j=0; j<src->list[i]->size-1 && !found; j++){
                if( i != min_i && j != min_j ){
                    other = src->list[i]->getLineSegment(j);
                    if( other.p1->equals(shpoint) ){
                        unshared2 = other.p2;
                        found = true;
                    }else if( other.p2->equals(shpoint) ){
                        unshared2 = other.p1;
                        found = true;
                    }
                }
            }
        }

        //compute and compare angles
        double angle1 = angle3p(unshared1, shpoint, &qp),
               angle2 = angle3p(unshared2, shpoint, &qp);
        closest = angle1 < angle2 ? closest : other;
    }

    /*Finally we check to which side of "closest" "pt" lies and report depending on whether
     * we're comparing against an external or internal ring.*/
    if( isEdgeFromExtRing ){
        return (qp.sideOf(&closest) == RIGHT) ? true : false;
    }else{
        return (qp.sideOf(&closest) == LEFT) ? true : false;
    }
}

/*Returns all nodes that intersect a given poligon "pol", including the 
  polygon's interior. This means that this function returns all tree nodes that
  are strictly contained in the polygon.*/
void *Node::searchInterior (GIMS_Polygon *pol){

    if ( this->type != GRAY ) { //if the node is a leaf node
        list<Node *> *l = new list<Node *>;
        l->push_back(this);
        return l;
    
    }else{
        list<Node *> *retlist = new list<Node *>;
        
        char center_contained = -1;
        //iterate over the child nodes
        for( Quadrant q : quadrantList ) {

            //fetch all components of the polygon that intersect the son's square
            GIMS_Polygon *clipped = (GIMS_Polygon *)(pol->clipToBox(sons[q]->square));

            bool contained = false;
            if( clipped == NULL ){
                /*if there are no intersections, then either the node is stricly contained or
                 *strictly outside of the polygon. Since the father center point belongs to
                 *all 4 children, we can test only that point for polygon containment.*/
                if( center_contained == -1 ){
                    GIMS_Point p = this->square->getCenter();
                    contained = sons[q]->polygonContainsPoint(pol, &p);
                    center_contained = contained ? 1 : 0;
                }else if(center_contained == 1){
                    contained = true;
                }else{
                    contained = false;
                }
            }

            if( contained ){
                //if the node is contained, we can report it free of charge
                retlist->push_back(sons[q]);
            }else if( clipped != NULL ){
                //if the node is intersected we call recursively to its sons.
                list<Node *> *l = (list<Node *> *)sons[q]->searchInterior(clipped);
                delete clipped;
                if( l != NULL ){
                    retlist->insert( retlist->end(), l->begin(), l->end() );
                    delete l;
                }
            }
        }
        return retlist;
    }
}

void mergeDicts(list<GIMS_Geometry *> a, list<GIMS_Geometry *> b){
    a->insert(a->end(), b->begin(), b->end());
}

list<GIMS_Geometry *> *Node::clipDict(list<GIMS_Geometry *> dict){
    list<GIMS_Geometry *> clipped = NULL;
    GIMS_Geometry *partial;
    for(list<GIMS_Geometry *>::iterator it = dict->begin(); it != dict->end(); it++){
        if( (partial = (*it)->clipToBox(this->square)) ){
            if( clipped == NULL )
                clipped = new list<GIMS_Geometry *>;
            clipped->push_back(partial);
        }
    }
    return clipped;
}

/*Inserts geometry "geom" in the tree*/
void Node::insert ( list<GIMS_Geometry *> geom ) {

    depth++;
    if( depth > 100 ){
        fprintf(stderr, "max depth reached (100), not inserting. Look out for bugs...\n");
        depth--;
        return;
    }

    list<GIMS_Geometry *> *clipped = this->clipDict(geom);

    if (clipped == NULL) { //geometry to insert does not intersect this node
        depth--;
        return;
    }

    if ( this->type != GRAY ) { //node type is only gray when it's not a leaf
                                //we're therefore accessing leaf nodes in this block

        if (this->dictionary != NULL) { //merge clipped with the node's dictionary
            mergeDicts(clipped, this->dictionary);
        }

        if ( this->validateGeometry(clipped) ) {
            //if the geometry is a valid node geometry we insert it into the node
            if(this->dictionary != NULL)
                delete this->dictionary;
            this->dictionary = clipped;
            this->type = BLACK;
            depth--;
            return;
        } else {
            if(this->dictionary != NULL){
                delete this->dictionary;
                this->dictionary = NULL;
            }
            this->split();
        }
    }

    for (Quadrant q : quadrantList) { //recursive step
        this->sons[q]->insert ( clipped );
    }
    
    depth--;
    if( clipped->type != POINT )
        delete clipped;
}

/* Returns true if the given geometry is a valid one for the calling node
   !Note! The bounding box geometry is not supported.
   The behaviour is undefined in such a situation.!Note! */
bool Node::validateGeometry () {

    GIMS_Point *shared = NULL;

    for ( list<GIMS_Geometry *>::iterator it = this->dictionary->begin(); it != this->dictionary->end(); it++ ) {

        if ( (*it)->type == POINT ) {
            if(contained == NULL)
                contained = (GIMS_Point *)(*it);
            else{
                if( !((GIMS_Point *)(*it))->equals(contained) )
                    return false;
            }

        }else if ( (*it)->type == EDGE ) {

            bool p1Inside = ((GIMS_LineSegment *)(*it))->p1->isInsideBox( this->square ),
                 p2Inside = ((GIMS_LineSegment *)(*it))->p2->isInsideBox( this->square );

                if( p1Inside && contained != NULL && !((GIMS_LineSegment *)(*it))->p1->equals(contained) )
                    return false;
                if( p2Inside && contained != NULL && !((GIMS_LineSegment *)(*it))->p2->equals(contained) )
                    return false;
                if ( p1Inside && p2Inside )
                    return false;

                if( contained == NULL ){
                    if(p1Inside)
                        contained = ((GIMS_LineSegment *)(*it))->p1;
                    else if(p2Inside)
                        contained = ((GIMS_LineSegment *)(*it))->p2;
                }

        }else if( (*it)->type == POLYGON ){
            GIMS_Polygon *p = (GIMS_Polygon *)(*it);

            //validate the external ring
            for ( list<GIMS_Geometry *>::iterator ext = p->externalRing->list->begin(); ext != p->externalRing->list->end(); ext++ ){
                bool p1Inside = ((GIMS_LineSegment *)(*ext))->p1->isInsideBox( this->square ),
                     p2Inside = ((GIMS_LineSegment *)(*ext))->p2->isInsideBox( this->square );
                if( (contained != NULL && p1Inside && !((GIMS_LineSegment *)(*ext))->p1->equals(contained)) ||
                    (contained != NULL && p2Inside && !((GIMS_LineSegment *)(*ext))->p2->equals(contained)) ||
                    (p1Inside && p2Inside) ){
                    return false;
                }else if(contained == NULL){
                    if(p1Inside)
                        contained = ((GIMS_LineSegment *)(*ext))->p1;
                    if(p2Inside)
                        contained = ((GIMS_LineSegment *)(*ext))->p2;
                }
            }

            //iterate over the internal rings
            for ( list<GIMS_Geometry *>::iterator ir = p->internalRings->list->begin(); ir != p->internalRings->list->end(); ir++ ){
                //and validate each of those internal rings
                for( list<GIMS_Geometry *>::iterator edge = ((GIMS_GeometryList *)(*ir))->list->begin(); edge != ((GIMS_GeometryList *)(*ir))->list->end(); edge++ ){
                    bool p1Inside = ((GIMS_LineSegment *)(*edge))->p1->isInsideBox( this->square ),
                         p2Inside = ((GIMS_LineSegment *)(*edge))->p2->isInsideBox( this->square );
                    if( (contained != NULL && p1Inside && !((GIMS_LineSegment *)(*edge))->p1->equals(contained)) ||
                        (contained != NULL && p2Inside && !((GIMS_LineSegment *)(*edge))->p2->equals(contained)) ||
                        (p1Inside && p2Inside) )
                        return false;
                    else if(contained == NULL){
                        if(p1Inside)
                            contained = ((GIMS_LineSegment *)(*edge))->p1;
                        if(p2Inside)
                            contained = ((GIMS_LineSegment *)(*edge))->p2;
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
        GIMS_BoundingBox *square = 
            new GIMS_BoundingBox(
                new GIMS_Point( this->square->minx() + xlen * xmultiplier[q],
                                this->square->miny() + ylen * ymultiplier[q]
                              ),
                new GIMS_Point( this->square->minx() + xlen * (xmultiplier[q] + 0.5),
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
    this->dictionary = new list<GIMS_Geometry *>;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
    this->father = NULL;
}

Node::Node( GIMS_BoundingBox *square ){
    this->type = WHITE;
    this->square = square;
    this->dictionary = new list<GIMS_Geometry *>;
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




PMQuadTree::PMQuadTree (GIMS_BoundingBox *domain) {
    this->root = new Node( domain );
    this->query = NULL;
}

PMQuadTree::~PMQuadTree () {}



/*Functions that take care of the construction and maintenance of the structure*/
void PMQuadTree::build  (GIMS_Geometry *geom){}

void PMQuadTree::insert ( GIMS_Geometry *geom ) {
    this->root->insert(geom);
}

void PMQuadTree::remove (GIMS_Geometry *geom){

}

/*return all leaf nodes that intersect geom*/
void *PMQuadTree::search (GIMS_Geometry *geom){
    return this->root->search(geom);
}



/*Follow the operations between the data structure and a given geometry*/
RelStatus PMQuadTree::intersects_g  ( GIMS_Geometry *result, GIMS_Geometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::meets_g       ( GIMS_Geometry *result, GIMS_Geometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::contains_g    ( GIMS_Geometry *result, GIMS_Geometry *geom){
    return UNDECIDED_RELATIONSHIP;
}

RelStatus PMQuadTree::isContained_g ( GIMS_Geometry *result, GIMS_Geometry *geom){
    return UNDECIDED_RELATIONSHIP;
}



/*Retrieve all geometry elements that are partially or totally contained
  in a given bounding box*/
RelStatus PMQuadTree::isBoundedBy ( GIMS_Geometry *result, GIMS_BoundingBox *box){
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
            results = (list<Node *> *)(this->root->searchInterior( (GIMS_Polygon *)(this->query) ));
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
    for(list<GIMS_Geometry *>::iterator it = renderQueue->begin(); it != renderQueue->end(); it++){
        renderer->renderGeometry(cr, *it);
    }

    cr->set_source_rgba(0.73, 0.19, 0.03, 0.5);
    for(list<GIMS_Geometry *>::iterator it = redRenderQueue->begin(); it != redRenderQueue->end(); it++){
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

    //renderer->renderGeometry(cr, n->square);

    if( n->dictionary == NULL )
        return;

    if (n->type == BLACK) { //the WHITE type stands for empty node, thus we ignore it.
        for ( list<GIMS_Geometry *>::iterator it = n->dictionary->begin();
              it != n->dictionary->end(); it++ ) {

            if( (*it)->renderCount >= renderer->renderCount )
                continue;

            if ( (*it)->type == EDGE ) {
                //GIMS_LineSegment *trimmed =  ((GIMS_LineSegment*)(*it))->trimToBBox(n->square);
                //renderer->renderGeometry( cr, trimmed );
                //delete trimmed;
                renderer->renderGeometry( cr, *it );
                (*it)->renderCount++;
            }else if( (*it)->type == POINT ) {
                renderer->renderGeometry( cr, (GIMS_Point*)(*it) );
                (*it)->renderCount++;
            }else if( (*it)->type == POLYGON ) {
                (*it)->renderCount++;
                renderer->renderGeometry( cr, ((GIMS_Polygon *)(*it))->externalRing );
                cr->set_source_rgb(0.69, 0.19, 0.0);
                renderer->renderGeometry( cr, ((GIMS_Polygon *)(*it))->internalRings );
                cr->set_source_rgb(0.0, 0.0, 0.0);
            }
        }
    }
}

void PMQuadTree::onClick( double x, double y){
    printf("begin click event\n");

    /*
    GIMS_Polygon *pol= (GIMS_Polygon *)(((GIMS_GeometryList *)(this->query))->list->front());
    GIMS_Point *pt = new GIMS_Point(x,y);
    renderEdge = true;
    Node *n = ((list<Node *> *)(this->root->search(pt)))->front();
    n->polygonContainsPoint(pol, pt);
    renderEdge = false;
    */

    printf("finished click event\n");
}
