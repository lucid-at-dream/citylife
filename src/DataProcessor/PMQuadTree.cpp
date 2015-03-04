#include "PMQuadTree.hpp"

using namespace GIMS_GEOMETRY;
using namespace PMQUADTREE;
using namespace std;

Quadrant quadrantList[4] =  {NW, NE, SE, SW};
char quadrantLabels[4][3] =  {"NW", "NE", "SE", "SW"};
double xmultiplier[4] =  {0.0, 0.5, 0.5, 0.0};
double ymultiplier[4] =  {0.5, 0.5, 0.0, 0.0};
int depth = 0, maxDepth = 0, nnodes = 1;

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
   
    GIMS_Geometry *clipped = geom->clipToBox(this->square);
    if( clipped == NULL )
        return NULL;

    list<Node *> *retlist = new list<Node *>;
    
    if(this->type == GRAY){
        for( Quadrant q : quadrantList ) {
            list<Node *> *l = (list<Node *> *)sons[q]->search(clipped);
            if( l != NULL ){
                retlist->insert( retlist->end(), l->begin(), l->end() );
                delete l;
            }
        }
    }else{
        retlist->push_back(this);
    }
    
    clipped->deleteClipped();
    return retlist;

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
    list<Node *> *nodes = (list<Node *> *)(cNode->search( &pt ));
    Node *northern = nodes->front();
    delete nodes;
    return northern;
}

/*if node has reference to a geometry labeled with id, returns the geometry's 
  reference, else returns NULL*/
GIMS_Geometry *Node::hasReferenceTo( long id ){
    if( this->dictionary == NULL )
        return NULL;

    for(list<GIMS_Geometry *>::iterator it = this->dictionary->begin();
        it != this->dictionary->end(); it++){
        if( (*it)->id != 0 && (*it)->id == id )
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
        if( n == NULL ){
            return false;
        }
    }

    //set up the query point
    GIMS_Point qp;
    if(this == n)
        qp = *pt;
    else
        qp = GIMS_Point(pt->x, n->square->lowerLeft->y);

    //The first portion of the polygon "pol" that we found going in the north direction
    GIMS_Polygon *p = (GIMS_Polygon *)first;

    /*from all the line segments that belong to polygon pol and that intersect 
      the node we just found, find out which one is closest to point pt.*/
    
    //keep track whether the closest edge is from an internal ring or the external
    //ring, as it will affect the side the point should be on to be contained.
    bool isEdgeFromExtRing = false;
    double minDist = 1e100;
    double tmp;
    GIMS_LineSegment closest;

    //iterate over the edges from the polygon rings
    GIMS_MultiLineString *prings[2] = {p->externalRing, p->internalRings};

    for(GIMS_MultiLineString *ring : prings){
        for(int i=0; ring != NULL && i<ring->size; i++){
            for(int j=0; j<ring->list[i]->size-1; j++){
                GIMS_LineSegment curr = ring->list[i]->getLineSegment(j);
                if( (tmp = distToSegmentSquared(&qp, &curr)) < minDist ){
                    minDist = tmp;
                    closest = curr;
                    isEdgeFromExtRing = (ring == p->externalRing);
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

    GIMS_MultiLineString *src = isEdgeFromExtRing ? p->externalRing : p->internalRings;

    bool p1inside = closest.p1->isInsideBox(n->square);
    GIMS_LineSegment auxls;

    GIMS_Point *shpoint = p1inside ? closest.p1 : closest.p2;
    GIMS_Point *unshared1 = p1inside ? closest.p2 : closest.p1;
    GIMS_Point *unshared2 = NULL;
    GIMS_LineSegment other;

    //find the edge that shares the endpoint with current "closest"
    bool found = false;
    int i,j;
    for(i=0; i<src->size && !found; i++){
        for(j=0; j<src->list[i]->size-1 && !found; j++){
            other = src->list[i]->getLineSegment(j);
            if( other.p1->equals(shpoint) && !other.p2->equals(unshared1) ){
                unshared2 = other.p2;
                found = true;
            }else if( other.p2->equals(shpoint) && !other.p1->equals(unshared1) ){
                unshared2 = other.p1;
                found = true;
            }
        }
    }

    if( unshared2 != NULL ){
        //compute and compare angles
        double angle1 = angle3p(unshared1, shpoint, &qp),
               angle2 = angle3p(unshared2, shpoint, &qp);

        auxls = angle1 >= angle2 ? other : closest;
        closest = angle1 < angle2 ? closest : other;
    }

    /*Finally we check to which side of "closest" "pt" lies and report depending on whether
     * we're comparing against an external or internal ring.*/
    if( isEdgeFromExtRing ){
        return (qp.sideOf(&closest) != LEFT) ? true : false;
    }else{
        return (qp.sideOf(&closest) != RIGHT) ? false : true;
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
        list<Node *> *retlist = new list<Node *>();
        
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
                list<Node *> *l = (list<Node *> *)(sons[q]->searchInterior(clipped));
                if( l != NULL ){
                    retlist->insert( retlist->end(), l->begin(), l->end() );
                    delete l;
                }
                clipped->deleteClipped();
            }
        }
        return retlist;
    }
}

list<GIMS_Geometry *> *Node::unconstrainedActiveSearch(int(*filter)(GIMS_Geometry *)){
    list<GIMS_Geometry *> *results = new list<GIMS_Geometry *>();
    
    if( this->type != GRAY ){
        if(this->dictionary != NULL){
            for(list<GIMS_Geometry*>::iterator it = dictionary->begin(); it != dictionary->end(); it++){
                if(filter(*it))
                    results->push_back(*it);
            }
        }
    }else{
        for( Quadrant q : quadrantList ){
            list<GIMS_Geometry *> *partial = sons[q]->unconstrainedActiveSearch(filter);
            results->insert(results->end(), partial->begin(), partial->end());
            delete partial;
        }
    }
    return results;
}

/*Returns all geometries stored in nodes intersected by polygon pol that pass the
  filter function test*/
list<GIMS_Geometry *> *Node::activeInteriorSearch (GIMS_Polygon *pol, 
                                                    int(*intersectedFilter)(Node *, GIMS_Geometry *, GIMS_Geometry *),
                                                    int(*containedFilter)(GIMS_Geometry *) ){

    list<GIMS_Geometry *> *results = new list<GIMS_Geometry *>();
    if ( this->type != GRAY ) { //if the node is a leaf node
        if(this->dictionary != NULL){
            for(list<GIMS_Geometry*>::iterator it = dictionary->begin(); it != dictionary->end(); it++){
                if(intersectedFilter(this, pol, *it))
                    results->push_back(*it);
            }
        }
        return results;
    
    }else{
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
                
                }

                if(center_contained == 1){
                    list<GIMS_Geometry *> *partial = sons[q]->unconstrainedActiveSearch(containedFilter);
                    results->insert(results->end(), partial->begin(), partial->end());
                    delete partial;
                }

            }else{
                //if the node is intersected we call recursively to its sons.
                list<GIMS_Geometry *> *partial = sons[q]->activeInteriorSearch(clipped, intersectedFilter, containedFilter);
                results->insert( results->end(), partial->begin(), partial->end() );
                delete partial;
                clipped->deleteClipped();
            }
        }
        return results;
    }
}

void mergeDicts(list<GIMS_Geometry *> *a, list<GIMS_Geometry *> *b){
    a->insert(a->end(), b->begin(), b->end());
}

list<GIMS_Geometry *> *Node::clipDict(list<GIMS_Geometry *> *dict){
    list<GIMS_Geometry *> *clipped = NULL;
    GIMS_Geometry *partial;
    for(list<GIMS_Geometry *>::iterator it = dict->begin(); it != dict->end(); it++){
        if( (partial = (*it)->clipToBox(this->square)) != NULL ){
            if( clipped == NULL )
                clipped = new list<GIMS_Geometry *>();
            clipped->push_back(partial);
        }
    }
    return clipped;
}

/*Inserts geometry "geom" in the tree*/
void Node::insert ( list<GIMS_Geometry *> *geom ) {
    depth++;

    if(depth > maxDepth)
        maxDepth = depth;

    if( depth > 200 ){
        fprintf(stderr, "max depth reached (200), not inserting. Look out for bugs...\n");
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
            delete this->dictionary;
            this->dictionary = NULL;
        }

        if ( this->validate(clipped) ) {
        //if(this->numPoints(clipped) <= 50) {
            //if the geometry is a valid node geometry we insert it into the node
            this->dictionary = clipped;
            this->type = BLACK;
            depth--;
            return;
        } else {
            this->split();
        }
    }

    for (Quadrant q : quadrantList) { //recursive step
        this->sons[q]->insert ( clipped );
    }
    
    for(list<GIMS_Geometry *>::iterator it = clipped->begin(); it != clipped->end(); it++)
        (*it)->deleteClipped();
    delete clipped;
    depth--;
}

/* Returns true if the given geometry is a valid one for the calling node
   !Note! The bounding box geometry is not supported.
   The behaviour is undefined in such a situation.!Note! */
bool Node::validate (list<GIMS_Geometry *> *dict) {
    GIMS_Point *sharedPoint = NULL;
    for ( list<GIMS_Geometry *>::iterator it = dict->begin(); it != dict->end(); it++ ) {
        if( !this->validateGeometry((*it), &sharedPoint) )
            return false;
    }
    return true;
}

int Node::numPoints(list<GIMS_Geometry *> *dict){
    int total = 0;
    for(list<GIMS_Geometry *>::iterator it = dict->begin(); it != dict->end(); it++){
        total += (*it)->getPointCount();
    }
    return total;
}

bool Node::validateGeometry (GIMS_Geometry *g, GIMS_Point **sharedPoint){
    switch(g->type){
        case POINT:{
            if( !this->validatePoint( (GIMS_Point *)g, sharedPoint ) )
                return false;
            break;
        }
        case LINESTRING:
        case RING:{
            if( !this->validateLineString((GIMS_LineString *)g, sharedPoint) )
                return false;
            break;
        }
        case POLYGON:{
            if( !this->validatePolygon( (GIMS_Polygon *)g, sharedPoint ) )
                return false;
            break;
        }
        case MULTIPOINT:{
            GIMS_MultiPoint *mpt = (GIMS_MultiPoint*)g;
            for(int i=0; i<mpt->size; i++){
                if( !this->validatePoint(mpt->list[i], sharedPoint) )
                    return false;
            }
        }
        case MULTILINESTRING:{
            GIMS_MultiLineString *mls = (GIMS_MultiLineString *)g;
            for(int i=0; i<mls->size; i++){
                if( !this->validateLineString(mls->list[i], sharedPoint) )
                    return false;
            }
            break;
        }
        case MULTIPOLYGON:{
            GIMS_MultiPolygon *mp = (GIMS_MultiPolygon *)g;
            for(int i=0; i<mp->size; i++){
                if( !this->validatePolygon(mp->list[i], sharedPoint) )
                    return false;
            }
            break;
        }
        case GEOMETRYCOLLECTION:{
            GIMS_GeometryCollection *gc = (GIMS_GeometryCollection *)g;
            for(int i=0; i<gc->size; i++){
                if( !this->validateGeometry(gc->list[i], sharedPoint) )
                    return false;
            }
            break;
        }
        case LINESEGMENT:{
            if( !this->validateLineSegment( (GIMS_LineSegment *)g, sharedPoint ) )
                return false;
            break;
        }
        default:{
            fprintf(stderr, "unsupported geometry was passed on to the "
                    "node validation function.\n" );
            return true;
        }
    }
    return true;
}

bool Node::validatePoint( GIMS_Point *pt, GIMS_Point **sharedPoint ){
    if( *sharedPoint == NULL)
        *sharedPoint = pt;
    else if( !pt->equals(*sharedPoint) )
        return false;
    return true;
}

bool Node::validateLineString( GIMS_LineString *ls, GIMS_Point **sharedPoint ){
    for(int i=0; i<ls->size; i++){
        if(ls->list[i]->isInsideBox(this->square))
            if(!this->validatePoint(ls->list[i], sharedPoint))
                return false;
    }
    return true;
}

bool Node::validateLineSegment(GIMS_LineSegment *l, GIMS_Point **sharedPoint){
    bool p1Inside = l->p1->isInsideBox( this->square ),
         p2Inside = l->p2->isInsideBox( this->square );

    if ( p1Inside && p2Inside )
        return false;
    if( p1Inside && *sharedPoint != NULL && !l->p1->equals(*sharedPoint) )
        return false;
    if( p2Inside && *sharedPoint != NULL && !l->p2->equals(*sharedPoint) )
        return false;
    if( *sharedPoint == NULL )
        *sharedPoint = p1Inside ? l->p1 : (p2Inside ? l->p2 : NULL);
    return true;
}

bool Node::validatePolygon(GIMS_Polygon *p, GIMS_Point **sharedPoint){
    GIMS_MultiLineString *rings[2] = {p->externalRing, p->internalRings};

    for(GIMS_MultiLineString *src : rings){
        for(int i=0; src!=NULL && i<src->size; i++){
            for(int j=0; j<src->list[i]->size; j++){
                GIMS_Point *aux = src->list[i]->list[j];
                bool auxInside = aux->isInsideBox( this->square );
                if( auxInside && *sharedPoint != NULL && !aux->equals(*sharedPoint) )
                    return false;
                if( auxInside && *sharedPoint == NULL )
                    *sharedPoint = aux;
            }
        }
    }
    return true;
}

/*creates four new sons (one for each quadrant) in the calling node*/
void Node::split(){
    nnodes += 4;
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
    this->dictionary = NULL;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
    this->father = NULL;
}

Node::Node( GIMS_BoundingBox *square ){
    this->type = WHITE;
    this->square = square;
    this->dictionary = NULL;
    this->sons[0] = this->sons[1] = this->sons[2] = this->sons[3] = NULL;
    this->father = NULL;
}

Node::~Node(){
    if(this->type == GRAY)
        for(Quadrant q: quadrantList)
            delete this->sons[q];

    this->square->deepDelete();

    if(dictionary != NULL){
        for(list<GIMS_Geometry *>::iterator it = dictionary->begin(); it != dictionary->end(); it++)
            (*it)->deleteClipped();
        delete dictionary;
    }
}
/*
Polygonal Map QuadTree Node<--
*/




PMQuadTree::PMQuadTree (GIMS_BoundingBox *domain) {
    this->root = new Node( domain );
    this->query = NULL;
}

PMQuadTree::~PMQuadTree () {
    delete this->root;
}

int PMQuadTree::getNumNodes(){
    return nnodes;
}
int PMQuadTree::getMaxDepth(){
    return maxDepth;
}


/*Functions that take care of the construction and maintenance of the structure*/
void PMQuadTree::build  (GIMS_Geometry *geom){}

void PMQuadTree::insert ( list<GIMS_Geometry *> *geom ){
    this->root->insert(geom);
}

void PMQuadTree::insert ( GIMS_Geometry *geom ) {
    list<GIMS_Geometry *> *aux = new list<GIMS_Geometry *>();
    aux->push_back(geom);
    this->root->insert(aux);
    delete aux;
}

void PMQuadTree::remove (GIMS_Geometry *geom){

}

/*return all leaf nodes that intersect geom*/
void *PMQuadTree::search (GIMS_Geometry *geom){
    return this->root->search(geom);
}



/*Follow the operations between the data structure and a given geometry*/
bool PMQuadTree::contains(GIMS_Geometry* container, GIMS_Geometry* contained){
    if(container->type == POLYGON && contained->type == POINT){
        list<Node *> *l = (list<Node *> *)(this->search(contained));
        bool res = l->front()->polygonContainsPoint((GIMS_Polygon *)container, (GIMS_Point *)contained);
        delete l;
        return res;
    }
    return false;
}

list<GIMS_Geometry *> *PMQuadTree::getRelated(GIMS_Geometry *g,
                                              int(*intersectedFilter)(Node *, GIMS_Geometry *, GIMS_Geometry *),
                                              int(*containedFilter)(GIMS_Geometry *)){
    if(g->type == POLYGON){
        return this->root->activeInteriorSearch((GIMS_Polygon *)g, intersectedFilter, containedFilter);
    }
    return NULL;
}

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
void PMQuadTree::renderRed ( GIMS_Geometry *g){
    redRenderQueue->push_back(g);
}

void PMQuadTree::renderGreen ( GIMS_Geometry *g){
    renderQueue->push_back(g);
}

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
        cr->stroke();
        cr->set_source_rgba(0.0, 0.19, 0.69, 0.2);
        for( list<Node *>::iterator i = results->begin(); i!= results->end(); i++ ){
            renderer->renderFilledBBox( cr, (*i)->square );
            cr->fill();
        }
    }else{
        printf("null query\n");
    }

    if( renderQueue->size() > 0 ){
        cr->stroke();
        cr->set_source_rgb(0.19, 0.73, 0.12);
        for(list<GIMS_Geometry *>::iterator it = renderQueue->begin(); it != renderQueue->end(); it++){
            renderer->renderGeometry(cr, *it);
        }
        cr->stroke();
    }

    if( redRenderQueue->size() > 0){
        cr->stroke();
        cr->set_source_rgb(0.73, 0.19, 0.03);
        for(list<GIMS_Geometry *>::iterator it = redRenderQueue->begin(); it != redRenderQueue->end(); it++){
            renderer->renderGeometry(cr, *it);
        }
        cr->stroke();
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
        for( list<GIMS_Geometry *>::iterator it = n->dictionary->begin();
            it != n->dictionary->end(); it++ ) {
                renderer->renderGeometry( cr, *it );
        }
    }
}

void PMQuadTree::onClick( double x, double y){
    printf("begin click event at %lf %lf\n", x, y);
    
    if(this->query == NULL)
        return;

    GIMS_Polygon *pol= (GIMS_Polygon *)(this->query);
    GIMS_Point *pt = new GIMS_Point(x,y);
    renderEdge = true;
    clock_t start = clock();
    Node *n = ((list<Node *> *)(this->root->search(pt)))->front();
    if( n->polygonContainsPoint(pol, pt) )
        renderQueue->push_back(pt);
    else
        redRenderQueue->push_back(pt);
    printf("took %.6lf cpu secs to label the point\n", (double)(clock() - start)/(double)CLOCKS_PER_SEC);
    renderEdge = false;
}
