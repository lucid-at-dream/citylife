#include "PMQuadTree.hpp"

using namespace GIMS_GEOMETRY;
using namespace PMQUADTREE;
using namespace std;

/*global variables initialization*/
Quadrant quadrantList[4] =  {NW, NE, SE, SW};
char quadrantLabels[4][3] =  {"NW", "NE", "SE", "SW"};
double xmultiplier[4] =  {0.0, 0.5, 0.5, 0.0};
double ymultiplier[4] =  {0.5, 0.5, 0.0, 0.0};

/*auxiliar variables for statistics*/
int depth = 0, maxDepth = 0, nnodes = 1;

unsigned long long pointcount = 0;

/*auxiliar variables for the debug rendering process.*/
bool renderEdge = false;
list<GIMS_Geometry *> *renderQueue = new list<GIMS_Geometry *>();
list<GIMS_Geometry *> *blackRenderQueue = new list<GIMS_Geometry *>();
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

    /*we then search for the point in the found node to get the containing leaf node.
    even tough there's no guarantee that only one node will be returned, there's a guarantee
    that all the retrieved nodes intersect the vertical line that goes through x*/
    list<Node *> *nodes = (list<Node *> *)(cNode->search( &pt ));
    Node *northern = nodes->front();
    delete nodes;
    return northern;
}

/*if node has reference to a geometry labeled with id, returns the geometry's 
  reference, else returns NULL*/
GIMS_Geometry *Node::hasReferenceTo( long long id ){
    if( this->dictionary == NULL )
        return NULL;

    for(list<GIMS_Geometry *>::iterator it = this->dictionary->begin();
        it != this->dictionary->end(); it++){
        if((*it)->id == id)
            return *it;
    }

    return NULL;
}

/*Returns 0 if the point is outside the polygon, 
  1 if it lies inside and 2 if it lies on the polygon's border.*/
char Node::indexedPolygonContainsPoint(GIMS_Polygon *pol, GIMS_Point *pt){
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
    if( !pt->isInsideBox( &(pol->bbox) ) )
        return 0;

    Node *n = this;
    GIMS_Geometry *first;

    while( (first = n->hasReferenceTo( pol->id )) == NULL ){
        n = n->goNorth( pt->x );
        if( n == NULL ){
            return 0;
        }
    }

    //set up the query point
    GIMS_Point qp;
    if( pt->isInsideBox(n->square) )
        qp = *pt;
    else
        qp = GIMS_Point(pt->x, n->square->lowerLeft->y);

    //The first portion of the polygon "pol" that we found going in the north direction
    GIMS_Polygon *p = (GIMS_Polygon *)first;

    /*from all the line segments that belong to polygon pol and that intersect 
      the node we just found, find out which one is closest to point pt.*/

    return p->containsPointWithinDomain(&qp, n->square);

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

            char contained = 0;
            if( clipped == NULL ){
                /*if there are no intersections, then either the node is stricly contained or
                 *strictly outside of the polygon. Since the father center point belongs to
                 *all 4 children, we can test only that point for polygon containment.*/
                if( center_contained == -1 ){
                    GIMS_Point p = this->square->getCenter();
                    contained = sons[q]->indexedPolygonContainsPoint(pol, &p);
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

void Node::unconstrainedActiveSearch(DE9IM *resultset, int(*filter)(GIMS_Geometry *)){
    if( this->type != GRAY ){
        if(this->dictionary != NULL){
            for(list<GIMS_Geometry *>::iterator it = dictionary->begin(); it != dictionary->end(); it++){
                if(filter(*it)){
                    
                    matrix_t::iterator m_it = resultset->setIntersect((*it)->id, dim(*it)); //we trivially now know that query and *it intersect
                    resultset->setII(m_it, dim(*it)); //we trivially now know that query's interior and *it's interior intersect 
                    
                    /*since *it border intersects this node which is contained in the query,
                    we can assume that there's at least a portion of *it's exterior intersecting
                    query's interior.*/
                    resultset->setIE(m_it, dim(*it));

                    /*since part of *it's border is inside the polygon, we know that, unless *it's
                      contained border is solely a polygon's interior ring, *it's exterior intersects query's border*/
                    if( //if it is not a polygon
                        (*it)->type != POLYGON
                        //or if it is a polygon containing external ring border
                        || ( (*it)->type == POLYGON && 
                           ( ((GIMS_Polygon *)(*it))->externalRing != NULL && ((GIMS_Polygon *)(*it))->externalRing->size > 0 ) )
                        ){
                        //then we can conclude the previous statement (just above the if clause)
                        resultset->setBE(m_it, MIN(borderDim(resultset->query), dim(*it)));
                    }
                }
            }
        }
    }else{
        for( Quadrant q : quadrantList ){
            sons[q]->unconstrainedActiveSearch(resultset, filter);
        }
    }
}

/*Returns all geometries stored in nodes intersected by polygon pol that pass the
  filter function test*/
void Node::activeInteriorSearch( DE9IM *resultset, GIMS_Polygon *query, int(*filter)(GIMS_Geometry *) ){
    
    if ( this->type != GRAY ) { //if the node is a leaf node
        if(this->dictionary != NULL){
            for(list<GIMS_Geometry *>::iterator it = dictionary->begin(); it != dictionary->end(); it++){
                if( filter(*it) ){
                    this->buildIM(resultset, query, *it);
                }
            }
        }
    
    }else{
        char center_contained = -1;
        //iterate over the child nodes
        for( Quadrant q : quadrantList ) {

            //fetch all components of the polygon that intersect the son's square
            GIMS_Polygon *clipped = (GIMS_Polygon *)(query->clipToBox(sons[q]->square));

            char contained = 0;
            if( clipped == NULL ){
                /*if there are no intersections, then either the node is stricly contained or
                 *strictly outside of the polygon. Since the father center point belongs to
                 *all 4 children, we can test only that point for polygon containment.*/
                if( center_contained == -1 ){
                    GIMS_Point p = this->square->getCenter();
                    contained = sons[q]->indexedPolygonContainsPoint(query, &p);
                    center_contained = contained ? 1 : 0;
                }

                if(center_contained == 1){
                    sons[q]->unconstrainedActiveSearch(resultset, filter);
                }

            }else{
                //if the node is intersected we call recursively to its sons.
                sons[q]->activeInteriorSearch(resultset, clipped, filter);
                clipped->deleteClipped();
            }
        }
    }
}

void Node::activeSearch(DE9IM *resultset, GIMS_Geometry *query, int(*filter)(GIMS_Geometry *)){
    
    GIMS_Geometry *clipped = query->clipToBox(this->square);
    if( clipped == NULL )
        return;
    
    if(this->type == GRAY){
        for( Quadrant q : quadrantList )
            sons[q]->activeSearch(resultset, clipped, filter);
    }else{
        //build the intersection matrix
        for(list<GIMS_Geometry *>::iterator it = this->dictionary->begin();
            it != this->dictionary->end(); it++){
            if(filter(*it) && (*it)->id != query->id){
                this->buildIM(resultset, query, *it);
            }
        }
    }
    
    clipped->deleteClipped();
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

void Node::remove (GIMS_Geometry *geom){
    /*TODO: implement remotion*/
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
            clipped->insert(clipped->end(), dictionary->begin(), dictionary->end());
            delete this->dictionary;
            this->dictionary = NULL;
        }

        //if ( this->validate(clipped) ) {
        if(this->numPoints(clipped) <= configuration.max_points_per_node) {
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




/*9 intersection model building functions*/
void Node::buildIM (DE9IM *resultset, GIMS_Geometry *query, GIMS_Geometry *other){
    if(query->type == POINT){
        buildIM_point(resultset, (GIMS_Point *)query, other);

    }else if(query->type == LINESTRING){
        GIMS_MultiLineString aux;
        aux.append((GIMS_LineString *)query);
        aux.id = query->id;
        buildIM_linestring(resultset, &aux, other);

    }else if(query->type == MULTILINESTRING){
        buildIM_linestring(resultset, (GIMS_MultiLineString *)query, other);

    }else if(query->type == POLYGON){
        buildIM_polygon(resultset, (GIMS_Polygon *)query, other);

    }else{
        perror("unsuported: topologicalSearch called on a multi geometry.");
    }
}

void DE9IM_pol_pol(DE9IM *resultset, GIMS_Polygon *A, GIMS_Polygon *B, GIMS_BoundingBox *domain);

void Node::buildIM_polygon(DE9IM *resultset, GIMS_Polygon *query, GIMS_Geometry *other){
    if(other->type == POINT){

        matrix_t::iterator it = resultset->setIE(other->id, 2);
        resultset->setBE(it, 1);

        //if the polygon lies inside or on the polygon's border
        char cnt;
        if( (cnt = this->indexedPolygonContainsPoint(query, (GIMS_Point *)other)) ){
            resultset->setIntersect(it, 0);
            //if the point doesn't lie on the polygon's border
            if(cnt == 1)
                resultset->setII(it, 0);
        }else{
            resultset->setEI(it, 0);
            resultset->setEB(it, 0);
        }

    }else if(other->type == LINESTRING){
TODO(BUILD_IM: polygon vs linestring)
        /*check if query intersects other*/
        /*check if query's interior intersects other's interior*/
        /*check if query's exterior intersects other's interior*/
        /*check if query's interior intersects other's exterior*/
        /*check if query's exterior intersects other's boundary*/
        /*check if query's boundary intersects other's exterior*/

    }else if(other->type == MULTILINESTRING){
TODO(BUILD_IM: polygon vs multilinestring)

    }else if(other->type == POLYGON){

        DE9IM_pol_pol(resultset, query, (GIMS_Polygon *)other, this->square);

        /*
        GIMS_Polygon *pol = (GIMS_Polygon *)other;
        if( pol->bbox.isDisjoint( query->bbox ) )
            return;

        BentleySolver bs;
        bs.polygon_polygon(resulset, query, pol);

        list<GIMS_Geometry *> ExtExt = bs.solve( query->externalRing , pol->externalRing  );
        //list<GIMS_Geometry *> ExtInt = bs.solve( query->externalRing , pol->internalRings );
        //list<GIMS_Geometry *> IntExt = bs.solve( query->internalRings, pol->externalRing  );
        //list<GIMS_Geometry *> IntInt = bs.solve( query->internalRings, pol->internalRings );

        //handle exterior ring vs. exterior ring intersections
        for( list<GIMS_Geometry *>::iterator it = ExtExt.begin(); it != ExtExt.end(); it++ ){
            if( (*it)->type == POINT ){
        
                int qind = query->externalRing->indexOf( (GIMS_Point *)(*it) ),
                    oind = other->externalRing->indexOf( (GIMS_Point *)(*it) )

                if( qind > -1 || oind > -1 ){
                    //if the intersection point belongs to one of the borders, we can conclude that
                    //their borders intersect with dim = 0 at this point.

                    GIMS_Point *o_p1 = other->externalRing->getPrevPoint(oind),
                               *o_p2 = (GIMS_Point *)(*it);
                               *o_p3 = other->externalRing->getNextPoint(oind);
                    if( o_p1 == NULL || o_p3 == NULL )
                        continue; //then this intersection will be handled in another node

                    GIMS_Point *q_p1 = query->externalRing->getPrevPoint(qind),
                               *q_p2 = (GIMS_Point *)(*it);
                               *q_p3 = query->externalRing->getNextPoint(qind);
                    if( q_p1 == NULL || q_p3 == NULL )
                        continue; //then this intersection will be handled in another node


                    //if one of o_p1 or o_p3 lie to the left side of both LINESEG( q_p1,q_p2 ) and LINESEG( q_p2,q_p3 )
                    GIMS_LineSegment ls1 = GIMS_LineSegment(q_p1, q_p2),
                                     ls2 = GIMS_LineSegment(q_p2, q_p3);

TODO(what if were looking at overlaping borders??)
                    
                    if( (o_p1->sideOf(&ls1) == LEFT && o_p1->sizeOf(&ls2) == LEFT) ||
                        (o_p2->sideOf(&ls1) == LEFT && o_p2->sizeOf(&ls2) == LEFT) ){
                        //if the lines associated with the point 
                        //lie to the internal side of the query polygon, then:
                        matrix_t::iterator matrix = resultset->setIntersect(other->id, 2);
                        resultset->setII(matrix, 2);
                        resultset->setIE(matrix, 2);
                        resultset->setBE(matrix, 1);
                    }else{
                        matrix_t::iterator matrix = resultset->setIntersect(other->id, 0);
                        resultset->setEI(matrix, 2);
                        resultset->setIE(matrix, 2);
                        resultset->setBE(matrix, 1);
                        resultset->setEB(matrix, 1);
                    }
                }else{
                    matrix_t::iterator matrix = resultset->setIntersect(other->id, 2);
                    resultset->setII(matrix, 2);
                    resultset->setEI(matrix, 2);
                    resultset->setIE(matrix, 2);
                    resultset->setBE(matrix, 1);
                    resultset->setEB(matrix, 1);
                }
            }else{ //it's a line segment
                //borders intersect with dimension 1
            }
        }
*/
    }
}


typedef struct _pointlist{
    int nnodes, allocated;
    GIMS_Point *nodes;
}pointlist;

typedef struct _pointmatrix{
    int nrows, allocated;
    pointlist *matrix;
}pointmatrix;

pointmatrix newPointMatrix(){
    pointmatrix matrix = {0,0,NULL};
    return matrix;
}

void addListToPointMatrix(pointmatrix &matrix, pointlist &ptlist){
    if( matrix.nrows + 1 > matrix.allocated ){
        matrix.matrix = (pointlist *)realloc(matrix.matrix, sizeof(pointlist) * (matrix.allocated+2));
        matrix.allocated += 2;
    }
    matrix.matrix[matrix.nrows++] = ptlist;
}

pointlist newPointList(){
    pointlist ptlist = {0,0,NULL};
    return ptlist;
}

void addPointToPointList(pointlist &ptlist, GIMS_Point *p){
    if(ptlist.nnodes + 1 > ptlist.allocated){
        ptlist.nodes = (GIMS_Point *)realloc(ptlist.nodes, sizeof(GIMS_Point) * (ptlist.allocated+3));
        ptlist.allocated+=3;
    }
    ptlist.nodes[ptlist.nnodes++] = *p;
}

GIMS_Point *__ptlist_cmp__reference__;

int __plist_cmp__(const void *_a, const void *_b){
    GIMS_Point *a = (GIMS_Point *)_a,
               *b = (GIMS_Point *)_b;
    double d1 = distanceSquared2p(a, __ptlist_cmp__reference__),
           d2 = distanceSquared2p(b, __ptlist_cmp__reference__);

    return d1 < d2 ? -1 : d1 > d2 ? 1 : 0;
}

void sortPointListWithRegardToFirst(pointlist &ptlist){
    __ptlist_cmp__reference__ = ptlist.nodes;
    qsort(ptlist.nodes+1, ptlist.nnodes-1, sizeof(GIMS_Point), __plist_cmp__);
}

void removeRepeatedPoints(pointlist &ptlist){
    GIMS_Point *prev = ptlist.nodes;
    int cur_ind = 1;

    int initialNumNodes = ptlist.nnodes;

    for(int i=1; i<initialNumNodes; i++){
        if( !prev->equals(ptlist.nodes+i) ){
            //printf("diff: %.2lf %.2lf - %.2lf %.2lf\n", prev->x, prev->y, ptlist.nodes[i].x, ptlist.nodes[i].y);
            ptlist.nodes[cur_ind] = ptlist.nodes[i];
            cur_ind++;
        }else{
            //printf("equal: %.2lf %.2lf - %.2lf %.2lf\n", prev->x, prev->y, ptlist.nodes[i].x, ptlist.nodes[i].y);
            ptlist.nnodes--;
        }
        prev = ptlist.nodes+i;
    }
}

void mergePointList(pointlist &keeper, pointlist &other){
    
    bool addFirst;
    if( keeper.nnodes > 0 )
        addFirst = !keeper.nodes[keeper.nnodes-1].equals( other.nodes );
    else
        addFirst = true;

    int total = keeper.nnodes + other.nnodes;
    if(!addFirst)
        total -= 1;

    keeper.nodes = (GIMS_Point *)realloc(keeper.nodes, sizeof(GIMS_Point) * total);

    int j;
    for(int i=keeper.nnodes; i<total; i++){
        j=i-keeper.nnodes;
        if(!addFirst)
            j+=1;
        keeper.nodes[i] = other.nodes[j];
    }

    keeper.nnodes = total;
}


/* Creates a dcel representation of a planar graph considering the argument polygon and domain.*/
DCEL polygonAndDomainAsPlanarGraph(GIMS_Polygon *P, GIMS_BoundingBox *domain){

    GIMS_Point *uL = new GIMS_Point(domain->lowerLeft->x , domain->upperRight->y),
               *uR = new GIMS_Point(domain->upperRight->x, domain->upperRight->y),
               *lR = new GIMS_Point(domain->upperRight->x, domain->lowerLeft->y),
               *lL = new GIMS_Point(domain->lowerLeft->x , domain->lowerLeft->y);

    GIMS_LineSegment *clipPolygon[] = {
        new GIMS_LineSegment(uL, uR),
        new GIMS_LineSegment(uR, lR),
        new GIMS_LineSegment(lR, lL),
        new GIMS_LineSegment(lL, uL)
    };

    /*create a pointlist for each line segment of the clip square*/
    pointlist clipPolNodes[] = { newPointList(), newPointList(), newPointList(), newPointList() };

    /*add the first point of each segment to each pointlist*/
    for(int i=0; i<4; i++){
        addPointToPointList( clipPolNodes[i], clipPolygon[i]->p1 );
        addPointToPointList( clipPolNodes[i], clipPolygon[i]->p2 );
    }

    /* First step is to identify the graph nodes of the polygon and their connectivity. 
     * At the same time we keep track of the domain boundary nodes and their connectivity.
     * This will result in two sets of nodes, which will then have to be merged in order to
     * build the planar graph. After this we'll build a DCEL structure that represents the 
     * worked out planar graph and we will evaluate which faces are covered by both geometric
     * figures and which are not, resulting in the final polygon clipping, represented as a
     * planar graph stored using a DCEL structure. phew, that was long!*/
    pointmatrix pslgSequences = newPointMatrix();

    for(int ring = 0; ring < P->externalRing->size; ring++){
        GIMS_LineString *border = P->externalRing->list[ring];
        pointlist connectedSequence = newPointList();

        for(int l = 0; l < border->size-1; l++){

            GIMS_LineSegment ls = border->getLineSegment(l);
            pointlist intersections = newPointList();

            addPointToPointList( intersections, border->list[l] );
            for(int l2 = 0; l2 < 4; l2++){
                
                GIMS_LineSegment *edge = clipPolygon[l2];

                GIMS_Geometry *it = ls.intersects(edge);
                /*if there is an intersection*/
                if(it != NULL){
                    /*consider the case where the intersection is a point*/
                    if(it->type == POINT){
                        addPointToPointList( intersections, (GIMS_Point *)it );
                        addPointToPointList( clipPolNodes[l2], (GIMS_Point *)it );
                    /*consider the case where the intersection is a line segment*/
                    }else{
                        addPointToPointList( intersections, ((GIMS_LineSegment *)it)->p1 );
                        addPointToPointList( intersections, ((GIMS_LineSegment *)it)->p2 );
                        addPointToPointList( clipPolNodes[l2], ((GIMS_LineSegment *)it)->p1 );
                        addPointToPointList( clipPolNodes[l2], ((GIMS_LineSegment *)it)->p2 );
                    }
                }
            }
            addPointToPointList( intersections, border->list[l+1] );

            /* Now that we know all intersection points that this edge of the polygon has 
             * with the domain boundary, we can sort them and the order will give us the 
             * adjacency between nodes.*/
            sortPointListWithRegardToFirst(intersections);
            removeRepeatedPoints( intersections );
            
            /* Now that we known all the graph nodes that lie on this edge of the polygon 
             * and the respective order, we need to merge this info with the rest of it 
             * in order to build a meaningful sequence for the entire polygon*/
            mergePointList(connectedSequence, intersections);
        }

        /* The planar graph of the polygon and the domain boundary may have several 
         * connected sequences, therefore we need to keep track of them in separate 
         * lists, given that the order is what identifies connectivity between nodes
         * and two connected sequences are disconnected from one another.*/
        addListToPointMatrix( pslgSequences, connectedSequence );
    }

    /*now we create the planar graph cycle for the clip polygon*/
    pointlist clipPolCycle = newPointList();
    for(int i=0; i<4; i++){
        sortPointListWithRegardToFirst(clipPolNodes[i]);
        removeRepeatedPoints( clipPolNodes[i] );
        mergePointList(clipPolCycle, clipPolNodes[i]);
    }

    /* With the gathered information we now have two planar graphs:
     * 1. A planar graph representing the polygon and its intersections with the clip polygon 
     * 2. A planar graph representing the clip polygon and its intersections with the polygon.
     * The combination of these two graphs gives us the whole planar graph, which can then be
     * represented using a DCEL. Therefore, the next steps of the algorithm take care of 
     * merging these two graphs in a DCEL data structure, which will have enough information
     * to create a clipping with enough information for building the DE-9IM.*/

    /* To do this merging we first start by finding the set of unique vertexes that compose 
     * the DCEL, associating with each vertex information about whether it belongs to the 
     * clip polygon or to the polygon being clipped.
     * Next, for each edge we create two directed halfedges. It is possible to know to which
     * geometric figure the edges belong to by using the related information on the vertexes.
     * Finally, we find the faces present in the planar subdivision and keep only enough 
     * information in the DCEL so that only the faces enclosed by both geometries are kept.*/

    /* Side note to self: 
     * - It's in moments like this, when you think you've got it all figured out, that you get screwed up the most!
     * - That's just life Bob...*/

    DCEL dcel;
    for(int i=0; i<pslgSequences.nrows; i++){
        for(int j=0; j<pslgSequences.matrix[i].nnodes; j++){
            GIMS_Point *p = pslgSequences.matrix[i].nodes+j;
            
            vertex *v = new vertex(); 
            v->pt = p;

            vertex *it = dcel.findVertex(v);
            if( it != NULL ){
                it->data |= 1;
                delete v;
            }else{
                dcel.addVertex(v);
            }
        }
    }

    for(int i=0; i<clipPolCycle.nnodes; i++){
        GIMS_Point *p = clipPolCycle.nodes+i;

        vertex *v = new vertex();
        v->pt = p;

        vertex *it = dcel.findVertex(v);
        if( it != NULL ){
            it->data |= 2;
            delete v;
        }else
            dcel.addVertex(v);
    }

    /*now that we have the list of vertexes, we need to find the set of halfedges of the DCEL*/
    for(int i=0; i<pslgSequences.nrows; i++){
        for(int j=0; j<pslgSequences.matrix[i].nnodes-1; j++){
            GIMS_Point *p1 = pslgSequences.matrix[i].nodes+j;
            GIMS_Point *p2 = pslgSequences.matrix[i].nodes+j+1;
            
            vertex *it_v1 = dcel.findVertex(p1);
            vertex *it_v2 = dcel.findVertex(p2);

            /*since there aren't repeated edges within the same graph, we can add them right away.*/
            halfedge *cw  = new halfedge();
            halfedge *ccw = new halfedge();

            cw->twin  = ccw;
            cw->tail  = it_v1;
            cw->data |= 2;
            it_v1->incidentEdges.push_back(cw);

            ccw->twin  = cw;
            ccw->tail  = it_v2;
            ccw->data |= 1;
            it_v2->incidentEdges.push_back(ccw);

            dcel.addHalfedge(cw);
            dcel.addHalfedge(ccw);
        }
    }

    for(int i=0; i<clipPolCycle.nnodes-1; i++){
        GIMS_Point *p1 = clipPolCycle.nodes+i;
        GIMS_Point *p2 = clipPolCycle.nodes+i+1;

        vertex *it_v1 = dcel.findVertex(p1);
        vertex *it_v2 = dcel.findVertex(p2);

        halfedge *cw  = new halfedge();
        halfedge *ccw = new halfedge();

        cw->twin  = ccw;
        cw->tail  = it_v1;
        cw->data |= 8;

        ccw->twin  = cw;
        ccw->tail  = it_v2;
        ccw->data |= 4;

        halfedge *it1 = dcel.findHalfedge(cw);
        halfedge *it2 = dcel.findHalfedge(ccw);
        
        /*if the edge also belongs to the polygon being clipped...*/
        if( it1 != NULL ){
            it1->data |= 8;
            delete cw;
        }else{
            dcel.addHalfedge(cw);
            it_v1->incidentEdges.push_back(cw);
        }

        if( it2 != NULL ){
            it2->data |= 4;
            delete ccw;
        }else{
            dcel.addHalfedge(ccw);
            it_v2->incidentEdges.push_back(ccw);
        }
    }

    /* for each vertex, sort the incident edges clockwise. For each pair of incident edges 
     * e1,e2 in clockwise order, assign e1->twin->next = e2 and e2->prev = e1->twin */
    for( vertexlist::iterator it = dcel.vertexes.begin(); it != dcel.vertexes.end(); it++ ){
        (*it)->incidentEdges.sort( sort_clockwise_cmp_ );

        list<halfedge *>::iterator prev_halfedge, next_halfedge;

        for( prev_halfedge = (*it)->incidentEdges.begin();
             prev_halfedge != (*it)->incidentEdges.end();
             prev_halfedge++ ){
    
            /* given that prev_halfedge stands for the previously discussed "e1", we must now
             * get "e2", which is the halfedge following "e2". Note that we also want to link the
             * last halfedge with the first halfedge.*/
            next_halfedge = list<halfedge *>::iterator( prev_halfedge );
            next_halfedge++;

            if( next_halfedge == (*it)->incidentEdges.end() )
                next_halfedge = (*it)->incidentEdges.begin();

            (*prev_halfedge)->twin->next = *next_halfedge;
            (*next_halfedge)->prev = (*prev_halfedge)->twin;            
        }
    }

    /* finally, given that the list of vertexes and edges has been created, we can now 
     * use the calculateFaces function from the DCEL class to compute the DCEL's faces.*/
    dcel.calculateFaces();

    /* Now that we have the planar graph represented as a DCEL, for each face we verify
       which parts of the two geometries cover it */
    for(facelist::iterator it = dcel.faces.begin(); it != dcel.faces.end(); it++){

        face *f = *it;
        
        halfedge *e   = f->boundary;
        halfedge *aux = e->next;

        /* associate the data of the bounding edges with the face's data*/
        f->data |= e->data;
        while(aux != e){
            f->data |= aux->data;
            aux = aux->next;
        }

        /* if at this point we don't have information about the face's intersection with either
         * the polygon or the domain, further processing has to be done to determine it.*/
        if( !(f->data & (8+4)) ){
            aux = aux->next;
            while(aux != e){
                double x = aux->tail->pt->x + (aux->twin->tail->pt->x - aux->tail->pt->x) / 2.0,
                       y = aux->tail->pt->y + (aux->twin->tail->pt->y - aux->tail->pt->y) / 2.0;
                GIMS_Point middlepoint = GIMS_Point(x,y);

                if( middlepoint.isContainedInBox( domain ) ){
                    f->data |= 8;
                    break;
                }
                aux = aux->next;
            }

            /*if not inside, it's outside.*/
            if( !(f->data & 8) )
                f->data |= 4;
        }

        if( !(f->data & (2+1)) ){
            aux = aux->next;
            while(aux != e){
                double x = aux->tail->pt->x + (aux->twin->tail->pt->x - aux->tail->pt->x) / 2.0,
                       y = aux->tail->pt->y + (aux->twin->tail->pt->y - aux->tail->pt->y) / 2.0;
                GIMS_Point middlepoint = GIMS_Point(x,y);
                
                if( P->containsPointWithinDomain(&middlepoint, domain) ){
                    f->data |= 2;
                    break;
                }
                aux = aux->next;
            }

            /*if not inside, it's outside.*/
            if( !(f->data & 2) )
                f->data |= 1;
        }
    }

    return dcel;
}

GIMS_Polygon *clipPolygonInDCEL(DCEL planargraph){

    printf("==== POLYGON ====\n");

    GIMS_Polygon *clippedPolygon = new GIMS_Polygon();

    for(facelist::iterator it = planargraph.faces.begin(); it != planargraph.faces.end(); it++){
        face *f = *it;

        //if the face is covered by both the polygon and the domain
        if( f->data & 8 && f->data & 2 ){
            printf("---- face ----\n");

            halfedge *e   = f->boundary;
            halfedge *aux = e->next;

            GIMS_LineString *ring = new GIMS_LineString();
            ring->appendPoint( e->tail->pt );
            e->tail->pt->id = e->data & 2 ? 0 : 1;

            printf("%lf %lf (%d), ", e->tail->pt->x, e->tail->pt->y, e->tail->pt->id);

            while(aux != e){
                aux->tail->pt->id = aux->data & 2 ? 0 : 1;
                printf("%lf %lf (%d), ", aux->tail->pt->x, aux->tail->pt->y, aux->tail->pt->id);
                ring->appendPoint( aux->tail->pt );
                aux = aux->next;
            }

            GIMS_Point *lclone = new GIMS_Point(e->tail->pt->x, e->tail->pt->y);
            ring->appendPoint( lclone );
            clippedPolygon->appendExternalRing(ring);
            printf("\n");
        }
    }

    return clippedPolygon;
}

typedef set<vertex *, vertex_cmp> vertexlist;
typedef set<halfedge *, halfedge_cmp> halfedgelist;

/*returns a DCEL representing a planar graph of A and B bounded to the argument domain.*/
DCEL buildPlanarGraph(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB, GIMS_BoundingBox *domain){

    DCEL planargraph;

    /*1. clip polygons to domain*/
    GIMS_Polygon *clippedA = clipPolygonInDCEL( polygonAndDomainAsPlanarGraph(polygonA, domain) ),
                 *clippedB = clipPolygonInDCEL( polygonAndDomainAsPlanarGraph(polygonB, domain) );

    /*2. create a PSLG of the two polygons*/

    return planargraph;
}

void DE9IM_pol_pol(DE9IM *resultset, GIMS_Polygon *A, GIMS_Polygon *B, GIMS_BoundingBox *domain){
    

    DCEL planarStraightLineGraph = buildPlanarGraph(A, B, domain);

    /* With the calculated planar graph, it is now possible to draw conclusions about the
     * intersection matrix of the polygons. */
    bool hasCommonFace = false,
         hasCommonEdges = false,
         hasCommonVertexes = false,
         edgeOfPolygonACrossesPolygonB = false,
         edgeOfPolygonBCrossesPolygonA = false,
         polygonAUniquelyEnclosesAFace = false,
         polygonBUniquelyEnclosesAFace = false;

    for( facelist::iterator it = planarStraightLineGraph.faces.begin(); 
         it != planarStraightLineGraph.faces.end(); it++ ){
        face *f = *it;
        if( f->data & 2  && f->data & 8 )
            hasCommonFace = true;
        else if( f->data & 2  )
            polygonAUniquelyEnclosesAFace = true;
        else if( f->data & 8 )
            polygonBUniquelyEnclosesAFace = true;
    }

    for( vertexlist::iterator it = planarStraightLineGraph.vertexes.begin();
         it != planarStraightLineGraph.vertexes.end(); it++ ){
        vertex *v = *it;
        if( v->data & 1 && v->data & 2 )
            hasCommonVertexes = true;
    }

    for( halfedgelist::iterator it = planarStraightLineGraph.halfedges.begin();
         it != planarStraightLineGraph.halfedges.end(); it++ ){
        halfedge *he = *it;

        if( (he->data & (2+1)) && (he->data & (8+4)) ){
            hasCommonEdges = true;

        }else if( (he->data & (2+1)) ){
            if( (he->left->data & 8) && (he->twin->left->data & 8) )
                edgeOfPolygonACrossesPolygonB = true;

        }else if( (he->data & (8+4)) ){
            if( (he->left->data & 2) && (he->twin->left->data & 2) )
                edgeOfPolygonBCrossesPolygonA = true;
        }
    }

    matrix_t::iterator matrix_index = resultset->getMatrixIndex(B->id);
    
    if( hasCommonFace ){
        resultset->setII(matrix_index, 2);
        resultset->setIntersect(matrix_index, 2);
    }else{
        if( edgeOfPolygonACrossesPolygonB || edgeOfPolygonBCrossesPolygonA || hasCommonEdges )
            resultset->setIntersect(matrix_index, 1);
        else if( hasCommonVertexes )
            resultset->setIntersect(matrix_index, 0);
    }

    if( polygonAUniquelyEnclosesAFace ){
        resultset->setIE(matrix_index, 2);
        resultset->setBE(matrix_index, 1);
    }

    if( polygonBUniquelyEnclosesAFace ){
        resultset->setEI(matrix_index, 2);
        resultset->setEB(matrix_index, 1);
    }
}

//helper functions for the linestring intersection matrix construction
void DE9IM_mls_ls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_LineString *other){
    BentleySolver bs;

    GIMS_MultiLineString *other_mls = new GIMS_MultiLineString(1);
    other_mls->append(other);

    list<GIMS_Geometry *> intersections = bs.solve(query, other_mls);

    delete other_mls;

    GIMS_LineString *other_original = (GIMS_LineString *)(*(idIndex.find(other)));
    GIMS_Point *otherBorder[2] = {other_original->list[0], other_original->list[other_original->size-1]};
    
    GIMS_Geometry *query_original = (GIMS_LineString *)(*(idIndex.find(query)));
    GIMS_MultiPoint queryBorder;

    if( query_original->type == LINESTRING){
        GIMS_LineString *aux_ls = (GIMS_LineString *)query_original;
        queryBorder.append( aux_ls->list[0] );
        queryBorder.append( aux_ls->list[ aux_ls->size-1 ] );
    }else if( query_original->type == MULTILINESTRING ){
        GIMS_MultiLineString *aux_mls = (GIMS_MultiLineString *)query_original;
        for(int i=0; i<aux_mls->size; i++){
            queryBorder.append( aux_mls->list[i]->list[0] );
            queryBorder.append( aux_mls->list[i]->list[ aux_mls->list[i]->size - 1 ] );
        }
    }

    int intersectedBorders = 0;

    list<GIMS_LineSegment *> linesegments;

    int shift_inc = queryBorder.size;

    for(list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++){
        if( (*it)->type == LINESEGMENT ){
            
            linesegments.push_back((GIMS_LineSegment *)(*it));

            for(int i=0; i<queryBorder.size; i++){
                if( ((GIMS_LineSegment *)(*it))->coversPoint( queryBorder.list[i] ) )
                    intersectedBorders |= 1 << i;
            }

            for( int i=0; i<2; i++ ){
                if( ((GIMS_LineSegment *)(*it))->coversPoint( otherBorder[i] ) )
                    intersectedBorders |= 1 << (shift_inc+i);
            }

            resultset->setIntersect(other->id, 1);
            resultset->setII(other->id, 1);
        
        }else{
            resultset->setIntersect(other->id, 0);

            bool isBorder = false;
            for(int i=0; i<queryBorder.size; i++){
                if( ((GIMS_Point *)(*it))->equals(queryBorder.list[i]) ){
                    intersectedBorders |= 1 << i;
                    isBorder = true;
                }
            }
            for( int i=0; i<2; i++ ){
                if( ((GIMS_Point *)(*it))->equals(otherBorder[i]) ){
                    intersectedBorders |= 1 << (shift_inc+i);
                    isBorder = true;
                }
            }
            if( !isBorder )
                resultset->setII(other->id, 0);
        }
    }

    /*Here we check intersections of Borders with exteriors*/
    if( (intersectedBorders & (1 << shift_inc)) == 0 || (intersectedBorders & (1 << (shift_inc+1))) == 0 ){
        resultset->setEB(other->id, 0);
    }

    for(int i=0; i<queryBorder.size; i++){
        if( (intersectedBorders & (1 << i)) == 0 ){
            resultset->setBE(other->id, 0);
            break;
        }
    }


    /*Here we check intersections of exteriors with interiors*/
    bool queryContained = false,
         otherContained = false;

    if( linesegments.size() >= (unsigned)(query->size) ){
        //check if linesegments cover the query geometry entirely
        queryContained = query->isCoveredBy(linesegments);
    }

    if( linesegments.size() >= (unsigned)(other->size) ){
        //check if linesegments cover the other geometry entirely
        otherContained = other->isCoveredBy(linesegments, false);
    }

    if( !queryContained && !otherContained ){
        resultset->setEI(other->id, 1);
        resultset->setIE(other->id, 1);
    }else if( !queryContained && otherContained ){
        cout << other->osm_id << " is contained by " << query->osm_id << endl;
        resultset->setIE(other->id, 1);
    }else if( queryContained && !otherContained ){
        cout << query->osm_id << " is contained by " << other->osm_id << endl;
        resultset->setEI(other->id, 1);
    }

    for(list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
        (*it)->deepDelete();

}



void DE9IM_mls_mls(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_MultiLineString *other){
    BentleySolver bs; 
    list<GIMS_Geometry *> intersections = bs.solve(query, other);
    
    //retrieve other's border
    GIMS_Geometry *original = (*(idIndex.find(other)));
    GIMS_MultiPoint otherBorder;
    if(original->type == LINESTRING){
        GIMS_LineString *orig_ls = (GIMS_LineString *)original;
        otherBorder.append(orig_ls->list[0]);
        otherBorder.append(orig_ls->list[orig_ls->size-1]);

    }else if(original->type == MULTILINESTRING){
        GIMS_MultiLineString *orig_mls = (GIMS_MultiLineString *)original;
        for(int i=0; i<orig_mls->size; i++){
            otherBorder.append(orig_mls->list[i]->list[0]);
            otherBorder.append(orig_mls->list[i]->list[orig_mls->list[i]->size-1]);
        }
    }

    //retrieve query's border
    original = (*(idIndex.find(query)));
    GIMS_MultiPoint queryBorder;
    if(original->type == LINESTRING){
        GIMS_LineString *orig_ls = (GIMS_LineString *)original;
        queryBorder.append(orig_ls->list[0]);
        queryBorder.append(orig_ls->list[orig_ls->size-1]);

    }else if(original->type == MULTILINESTRING){
        GIMS_MultiLineString *orig_mls = (GIMS_MultiLineString *)original;
        for(int i=0; i<orig_mls->size; i++){
            queryBorder.append(orig_mls->list[i]->list[0]);
            queryBorder.append(orig_mls->list[i]->list[orig_mls->list[i]->size-1]);
        }
    }

    int intersectedBorders = 0;
    list<GIMS_LineSegment *> linesegments;

    int shift_inc = queryBorder.size;

    for(list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++){
        if( (*it)->type == LINESEGMENT ){
            
            linesegments.push_back((GIMS_LineSegment *)(*it));

            for( int i=0; i<queryBorder.size; i++ ){
                if( ((GIMS_LineSegment *)(*it))->coversPoint( queryBorder.list[i] ) )
                    intersectedBorders |= 1 << i;
            }
            for( int i=0; i<otherBorder.size; i++ ){
                if( ((GIMS_LineSegment *)(*it))->coversPoint( otherBorder.list[i] ) )
                    intersectedBorders |= 1 << (shift_inc+i);
            }

            resultset->setIntersect(other->id, 1);
            resultset->setII(other->id, 1);
        
        }else{
            resultset->setIntersect(other->id, 0);

            bool isBorder = false;
            for( int i=0; i<queryBorder.size; i++ ){
                if( ((GIMS_Point *)(*it))->equals(queryBorder.list[i]) ){
                    intersectedBorders |= 1 << i;
                    isBorder = true;
                }
            }
            for( int i=0; i<otherBorder.size; i++ ){
                if( ((GIMS_Point *)(*it))->equals(otherBorder.list[i]) ){
                    intersectedBorders |= 1 << (shift_inc+i);
                    isBorder = true;
                }
            }

            if( !isBorder )
                resultset->setII(other->id, 0);
        }
    }

    /*Here we check intersections of Borders with exteriors*/
    for(int i=0; i<otherBorder.size; i++){
        if( (intersectedBorders & (1 << (shift_inc+i))) == 0 ){
            resultset->setEB(other->id, 0);
            break;
        }
    }

    for(int i=0; i<queryBorder.size; i++){
        if( (intersectedBorders & (1 << i)) == 0 ){
            resultset->setBE(other->id, 0);
            break;
        }
    }

    /*Here we check intersections of exteriors with interiors*/
    bool queryContained = false,
         otherContained = false;

    if( linesegments.size() >= (unsigned)(query->size) ){
        //check if linesegments cover the query geometry entirely
        queryContained = query->isCoveredBy(linesegments);
    }

    int total = 0;
    for(int i=0; i<other->size; i++)
        total += other->list[i]->size;

    if( linesegments.size() >= (unsigned)(total) ){
        //check if linesegments cover the other geometry entirely
        otherContained = other->isCoveredBy(linesegments, false);
    }

    if( !queryContained && !otherContained ){
        resultset->setEI(other->id, 1);
        resultset->setIE(other->id, 1);
    }else if( !queryContained && otherContained ){
        resultset->setIE(other->id, 1);
    }else if( queryContained && !otherContained ){
        resultset->setEI(other->id, 1);
    }

    for(list<GIMS_Geometry *>::iterator it = intersections.begin(); it != intersections.end(); it++)
        (*it)->deepDelete();

}



void Node::buildIM_linestring(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_Geometry *other){
    if(other->type == POINT){

        resultset->setIE(other->id, 0);
        resultset->setBE(other->id, 0);

        GIMS_Point *point = (GIMS_Point *)other;  

        GIMS_Geometry *original = (GIMS_LineString *)(*(idIndex.find(query)));
        GIMS_MultiPoint queryBorder;

        if( original->type == LINESTRING){
            GIMS_LineString *aux_ls = (GIMS_LineString *)original;
            queryBorder.append( aux_ls->list[0] );
            queryBorder.append( aux_ls->list[ aux_ls->size-1 ] );
        }else if( original->type == MULTILINESTRING ){
            GIMS_MultiLineString *aux_mls = (GIMS_MultiLineString *)original;
            for(int i=0; i<aux_mls->size; i++){
                queryBorder.append( aux_mls->list[i]->list[0] );
                queryBorder.append( aux_mls->list[i]->list[ aux_mls->list[i]->size - 1 ] );
            }
        }

        /*if the point is contained in the linestring then they intersect*/
        bool contained = query->coversPoint(point);
        if( contained ){
            resultset->setIntersect(other->id, 0);

            /*if the point is contained in the linestring and the point is different 
              from the line string endpoints, then their interiors intersect.*/
            bool isBorder = false;
            for(int i=0; i<queryBorder.size; i++){
                if( point->equals(queryBorder.list[0]) ){
                    isBorder = true;
                    break;
                }
            }
            if( !isBorder )
                resultset->setII(other->id, 0);

        }else{
            /*at this point we known that the point and line string don't intersect*/
            resultset->setEI(other->id, 0);
            resultset->setEB(other->id, 0);
        }

    }else if(other->type == LINESTRING){
        DE9IM_mls_ls(resultset, query, (GIMS_LineString *)other);
    }else if(other->type == MULTILINESTRING){
        DE9IM_mls_mls(resultset, query, (GIMS_MultiLineString *)other);

    }else if(other->type == POLYGON){
TODO(BUILD_IM: linestring vs polygon)
    }
}

void Node::buildIM_point(DE9IM *resultset, GIMS_Point *query, GIMS_Geometry *other){
    if( resultset->matrix.find(other->id) != resultset->matrix.end() )
        return;

    if( other->type == POINT ){
        if( query->equals((GIMS_Point *)other)  ){
            resultset->setIntersect(other->id, 0);
            resultset->setII(other->id, 0);
        }else{
            resultset->setIE(other->id, 0);
            resultset->setBE(other->id, 0);
            resultset->setEI(other->id, 0);
            resultset->setEB(other->id, 0);
        }

    }else if( other->type == LINESTRING ){

        resultset->setEI(other->id, 0);
        resultset->setEB(other->id, 0);

        GIMS_LineString *ls = (GIMS_LineString *)other;  

        /*if the point is contained in the linestring then they intersect*/
        bool contained = ls->coversPoint(query);
        if( contained ){
            resultset->setIntersect(other->id, 0);

            /*if the point is contained in the linestring and the point is different 
              from the line string endpoints, then their interiors intersect.*/
            if( !query->equals(ls->list[0]) && !query->equals(ls->list[ls->size-1]) )
                resultset->setII(other->id, 0);

        }else{
            /*at this point we known that the point and line string don't intersect*/
            resultset->setIE(other->id, 0);
            resultset->setBE(other->id, 0);
        }

    }else if( other->type == MULTILINESTRING ){

        resultset->setEI(other->id, 0);
        resultset->setEB(other->id, 0);

        GIMS_MultiLineString *mls = (GIMS_MultiLineString *)other;

        GIMS_Geometry *original = *(idIndex.find(other));

        GIMS_MultiPoint border;
        if(original->type == LINESTRING){
            GIMS_LineString *orig_ls = (GIMS_LineString *)original;
            border.append(orig_ls->list[0]);
            border.append(orig_ls->list[orig_ls->size-1]);

        }else if(original->type == MULTILINESTRING){
            GIMS_MultiLineString *orig_mls = (GIMS_MultiLineString *)original;
            for(int i=0; i<orig_mls->size; i++){
                border.append(orig_mls->list[i]->list[0]);
                border.append(orig_mls->list[i]->list[orig_mls->list[i]->size-1]);
            }
        }

        bool contained = false;
        for(int i=0; i<mls->size; i++){
            contained = mls->list[i]->coversPoint(query);
            if( contained ){
                resultset->setIntersect(other->id, 0);

                /*if the point is contained in the linestring and the point is different 
                  from the line string endpoints, then their interiors intersect.*/
                for(int j=0; j<border.size; j++){
                    if( query->equals(border.list[j]) )
                        break;
                }
                resultset->setII(other->id, 0);
                break;
            }
        }

        if(!contained){
            /*at this point we known that the point and line string don't intersect*/
            resultset->setIE(other->id, 0);
            resultset->setBE(other->id, 0);
        }

    }else if( other->type == POLYGON ){

        matrix_t::iterator it = resultset->setEI(other->id, 2);
        resultset->setEB(it, 1);

        //if the point lies inside or on the polygon's border
        char cnt;
        if( (cnt = this->indexedPolygonContainsPoint((GIMS_Polygon *)other, query)) ){
            resultset->setIntersect(it, 0);
            //if the point doesn't lie on the polygon's border
            if( cnt == 1 )
                resultset->setII(it, 0);
        }else{
            resultset->setIE(it, 0);
            resultset->setBE(it, 0);
        }

    }else if( other->type == MULTIPOLYGON ){
        GIMS_MultiPolygon *mp = (GIMS_MultiPolygon *)other;

        resultset->setEI(other->id, 0);
        resultset->setEB(other->id, 0);

        //if the polygon lies inside or on the polygon's border
        GIMS_Polygon pol; pol.id = mp->id;
        char cnt;
        if( (cnt = this->indexedPolygonContainsPoint(&pol, (GIMS_Point *)query)) ){
            resultset->setIntersect(other->id, 0);
            //if the point doesn't lie on the polygon's border

            if(cnt == 1)
                resultset->setII(other->id, 0);
        }else{
            resultset->setIE(other->id, 0);
            resultset->setBE(other->id, 0);
        }
    }else{
        perror("someone is trying to build a 9-intersection matrix against a weird geometry type.");
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
void PMQuadTree::insert ( GIMS_Geometry *geom ) {
    list<GIMS_Geometry *> *aux = new list<GIMS_Geometry *>();
    aux->push_back(geom);
    this->root->insert(aux);
    delete aux;
}

void PMQuadTree::insert (list<GIMS_Geometry *> &geom){
    for(list<GIMS_Geometry *>::iterator it = geom.begin(); it != geom.end(); it++)
        idIndex.insert(*it);
    
    this->root->insert(&geom);
}

void PMQuadTree::insert ( list<GIMS_Geometry *> *geom ){    
    for(list<GIMS_Geometry *>::iterator it = geom->begin(); it != geom->end(); it++)
        idIndex.insert(*it);

    this->root->insert(geom);
}

void PMQuadTree::remove (GIMS_Geometry *geom){
    /*TODO: implement remotion*/
}

/*return all leaf nodes that intersect geom*/
void *PMQuadTree::search (GIMS_Geometry *geom){
    return this->root->search(geom);
}

DE9IM *PMQuadTree::topologicalSearch( GIMS_Geometry *query, int(*filter)(GIMS_Geometry *) ){
    DE9IM *resultset = new DE9IM(query);

    if( query->type == POLYGON || query->type == MULTIPOLYGON ){
        this->root->activeInteriorSearch(resultset, (GIMS_Polygon *)query, filter);
    }else{
        this->root->activeSearch(resultset, query, filter);
    }

    if(false){
        //since polygons might have topological relationships even though they're not in the same
        //node (containment), we need to point label at least one of the points in the query set.
        GIMS_Point *pt;
        switch(query->type){
        case POINT:
            pt = (GIMS_Point *)query; break;
        case LINESTRING:
        case RING:
            pt = ((GIMS_LineString *)query)->list[0]; break;
        case POLYGON:
            pt = ((GIMS_Polygon *)query)->externalRing->list[0]->list[0]; break;
        case MULTILINESTRING:
            pt = ((GIMS_MultiLineString *)query)->list[0]->list[0]; break;
        case MULTIPOLYGON:
            pt = ((GIMS_MultiPolygon *)query)->list[0]->externalRing->list[0]->list[0]; break;
        default:
            fprintf(stderr, "unsupported geometry was passed on to the topologicalSearch function.\n" );
            return resultset;
        }

        list<Node *> *nodes = (list<Node *> *)(this->search(pt));
        Node *n = nodes->front();
        delete nodes;

        while( (n = n->goNorth(pt->x)) != NULL ){
            
            if( n->dictionary == NULL )
                continue;

            for(list<GIMS_Geometry *>::iterator it = n->dictionary->begin(); it != n->dictionary->end(); it++ ){
                if( (*it)->type == POLYGON && filter(*it) ){
                    
                    //if the polygon has already been considered, ignore it.
                    if( resultset->matrix.find((*it)->id) != resultset->matrix.end() )
                        continue;
                    
                    matrix_t::iterator matrix = resultset->setEI((*it)->id, 2);
                    resultset->setEB(matrix, 1);

                    //if the point lies inside the polygon
                    if( n->indexedPolygonContainsPoint((GIMS_Polygon *)(*it), pt) > 0 ){
                        resultset->setIntersect(matrix, dim(query));
                        resultset->setII(matrix, dim(query));
                    }else{
                        resultset->setIE(matrix, dim(query));
                        resultset->setBE(matrix, borderDim(query));
                    }
                }
            }
        }
    }

    return resultset;
}









/* Functions for debug renderization module */
void PMQuadTree::dumpLevita(){
    this->dumpLevita_r(this->root);
}

void PMQuadTree::dumpLevita_r(Node *n){
    /*if it is a leaf node*/
    if (n->type != GRAY) {
        this->dumpLevita_leaf(n);
    } else {
        for (Quadrant q : quadrantList) {
            this->dumpLevita_r(n->sons[q]);
        }
    }
}

void PMQuadTree::dumpLevita_leaf(Node *n){

    GIMS_Point ll = *(n->square->lowerLeft),
          ur = *(n->square->upperRight);
    
    GIMS_Point ul = GIMS_Point(ll.x, ur.y),
          lr = GIMS_Point(ur.x, ll.y);

    printf("PMQT_P: %llu %lf %lf q\n", pointcount++, ll.x, ll.y);
    printf("PMQT_P: %llu %lf %lf q\n", pointcount++, ul.x, ul.y);
    printf("PMQT_P: %llu %lf %lf q\n", pointcount++, ur.x, ur.y);
    printf("PMQT_P: %llu %lf %lf q\n", pointcount++, lr.x, lr.y);

    printf("PMQT_L: %llu %llu q\n", pointcount - 4, pointcount - 3);
    printf("PMQT_L: %llu %llu q\n", pointcount - 3, pointcount - 2);
    printf("PMQT_L: %llu %llu q\n", pointcount - 2, pointcount - 1);
    printf("PMQT_L: %llu %llu q\n", pointcount - 1, pointcount - 4);
}

void PMQuadTree::renderRed ( GIMS_Geometry *g){
    redRenderQueue->push_back(g);
}

void PMQuadTree::renderBlack ( GIMS_Geometry *g){
    blackRenderQueue->push_back(g);
}

void PMQuadTree::renderGreen ( GIMS_Geometry *g){
    renderQueue->push_back(g);
}

void PMQuadTree::debugRender(Cairo::RefPtr<Cairo::Context> cr){

    renderer.setScale( 400.0/this->root->square->xlength(),
                        -400.0/this->root->square->ylength() );
    renderer.setTranslation( -this->root->square->lowerLeft->x,
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
            renderer.renderFilledBBox( cr, (*i)->square );
            cr->fill();
        }
        cr->stroke();
        
        cr->stroke();
        cr->set_source_rgba(0, 0, 0, 1);
        renderer.renderGeometry(cr, query);
        cr->stroke();

    }else{
        printf("null query\n");
    }

    if( renderQueue->size() > 0 ){
        for(list<GIMS_Geometry *>::iterator it = renderQueue->begin(); it != renderQueue->end(); it++){
            cr->stroke();
            cr->set_source_rgb(0.19, 0.73, 0.12);
            renderer.renderGeometry(cr, *it);
            cr->stroke();
        }
    }

    if( blackRenderQueue->size() > 0){
        for(list<GIMS_Geometry *>::iterator it = blackRenderQueue->begin(); it != blackRenderQueue->end(); it++){
            cr->stroke();
            cr->set_source_rgb(0.03, 0.19, 0.73);
            renderer.renderGeometry(cr, *it);
            cr->stroke();
        }
    }

    if( redRenderQueue->size() > 0){
        for(list<GIMS_Geometry *>::iterator it = redRenderQueue->begin(); it != redRenderQueue->end(); it++){
            cr->stroke();
            cr->set_source_rgb(0.73, 0.19, 0.03);
            renderer.renderGeometry(cr, *it);
            cr->stroke();
        }
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

    renderer.renderGeometry(cr, n->square);

    if( n->dictionary == NULL )
        return;

    if (n->type == BLACK) { //the WHITE type stands for empty node, thus we ignore it.
        for( list<GIMS_Geometry *>::iterator it = n->dictionary->begin();
            it != n->dictionary->end(); it++ ) {
            //renderer.renderGeometry( cr, *it );
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
    
    if( n->indexedPolygonContainsPoint(pol, pt) )
        renderQueue->push_back(pt);
    else
        redRenderQueue->push_back(pt);

    printf("took %.6lf cpu secs to label the point\n", (double)(clock() - start)/(double)CLOCKS_PER_SEC);
    renderEdge = false;
}
