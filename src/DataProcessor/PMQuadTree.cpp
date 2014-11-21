#include "PMQuadTree.hpp"

using namespace GIMSGEOMETRY;
using namespace PMQUADTREE;
using namespace std;

Quadrant quadrantList[4] =  {NW, NE, SE, SW};
char quadrantLabels[4][3] =  {"NW", "NE", "SE", "SW"};
double xmultiplier[4] =  {0.0, 0.5, 0.5, 0.0};
double ymultiplier[4] =  {0.5, 0.5, 0.0, 0.0};
int depth = 0;

/*
-->Polygonal Map QuadTree Node
*/
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

Node *Node::goNorth( double x ){

    if( this->father == NULL )
        return NULL;

    GIMSPoint *pt = this->square->upperRight->clone();

    pt->y += ERR_MARGIN * 4;
    pt->x = x;

    list<Node *> *nodes;
    Node *cNode = this->father;
    while( !pt->isInsideBox(cNode->square) ){
        if(cNode->father != NULL)
            cNode = cNode->father;
        else
            return NULL;
    }

    nodes = ((list<Node *> *)(cNode->search( pt )));

    if(nodes->size() == 0){
        printf("but couldn't find the point inside of it??? What the hell??\n");
        return NULL;
    }else{
        return nodes->front();
    }
}

GIMSGeometry *Node::hasReferenceTo( unsigned long int id ){

    if( this->dictionary == NULL )
        return NULL;

    if( this->dictionary->size() == 0 )
        return NULL;

    for( list<GIMSGeometry *>::iterator it = this->dictionary->begin(); it != this->dictionary->end(); it++ ){
        if( (*it)->id == id ){
            return *it;
        }
    }
    return NULL;
}


double distToSegmentSquared(GIMSPoint *p, GIMSEdge *e) {
    double lineSegLenSquared = (e->p1->x - e->p2->x) * (e->p1->x - e->p2->x) + (e->p1->y - e->p2->y) * (e->p1->y - e->p2->y);

    if (lineSegLenSquared == 0)
        return (p->x - e->p1->x) * (p->x - e->p1->x) + (p->y - e->p1->y) * (p->y - e->p1->y);

    //dot product over squared of length
    double t = ((p->x - e->p1->x) * (e->p2->x - e->p1->x) + (p->y - e->p1->y) * (e->p2->y - e->p1->y)) / lineSegLenSquared;
    if (t < 0) return (p->x - e->p1->x) * (p->x - e->p1->x) + (p->y - e->p1->y) * (p->y - e->p1->y);
    if (t > 1) return (p->x - e->p2->x) * (p->x - e->p2->x) + (p->y - e->p2->y) * (p->y - e->p2->y);

    double nearest_pt_x = e->p1->x + t * (e->p2->x - e->p1->x),
           nearest_pt_y = e->p1->y + t * (e->p2->y - e->p1->y);

    return (p->x - nearest_pt_x) * (p->x - nearest_pt_x) + (p->y - nearest_pt_y) * (p->y - nearest_pt_y);
}

bool Node::polygonContainsPoint(GIMSPolygon *pol, GIMSPoint *pt){

    /*find the first node in the north direction than intersects polygon pol*/
    Node *n = this;
    GIMSGeometry *first;

    while( (first = n->hasReferenceTo( pol->id )) == NULL ){
        n = n->goNorth( pt->x );
        if( n == NULL )
            return false;
    }

    GIMSPolygon *p = (GIMSPolygon *)first;

    /*from all the line segments that belong to polygon pol and that intersect 
      the node we just found, find out which one is closest to point pt.*/
    bool isEdgeFromExtRing = false;
    double minDist = 1e100;
    GIMSEdge *closest = NULL;
    GIMSEdge *closestPrev = NULL,
             *closestNext = NULL;

    GIMSEdge *prev = NULL;
    for( list<GIMSGeometry *>::iterator it = p->externalRing->list->begin();
         it != p->externalRing->list->end(); it++ ){
        if( distToSegmentSquared( pt, (GIMSEdge *)(*it) ) < minDist ){
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

    for( list<GIMSGeometry *>::iterator int_ring = p->internalRings->list->begin();
         int_ring != p->internalRings->list->end(); int_ring++ ){
        prev = NULL;
        for( list<GIMSGeometry *>::iterator it = ((GIMSGeometryList *)(*int_ring))->list->begin();
             it != ((GIMSGeometryList *)(*int_ring))->list->end(); it++ ){
            if( distToSegmentSquared( pt, (GIMSEdge *)(*it) ) < minDist ){
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
      We can the shared enpoint pt2.
      Given this convention, we must now check which of the two line segments 
      forms the smallest angle with the linesegment pt---p2. We then check to 
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
                printf("@polygonContainsPoint - something that wasn't supposed to happen happened!");
                exit(-1);
            }
        }
        GIMSPoint *unshared2 = e2->p1->equals(pt2) ? e2->p2 : e2->p1;

        double angle1 = angle3p(unshared1, pt2, pt),
               angle2 = angle3p(unshared2, pt2, pt);


        closest = angle1 < angle2 ? e1 : e2;
    }


    /*check which side of closest pt lies in*/
    if( isEdgeFromExtRing ){
        if( pt->sideOf(closest) == RIGHT )
            return true;
        else
            return false;
    }else{
        if( pt->sideOf(closest) == LEFT )
            return true;
        else
            return false;
    }

}

void *Node::searchInterior (GIMSPolygon *pol){

    if ( this->type != GRAY ) {
        list<Node *> *l = new list<Node *>;
        l->push_back(this);
        return l;
    }else{
        list<Node *> *retlist = new list<Node *>;
        
        GIMSPolygon *clipped;
        for( Quadrant q : quadrantList ) {
            
            clipped = (GIMSPolygon *)(pol->clipToBox(sons[q]->square));

            bool contained = false;
            if( clipped == NULL ){
                GIMSPoint *p = sons[q]->square->upperRight->clone();
                p->x -= 2 * ERR_MARGIN;
                contained = sons[q]->polygonContainsPoint( pol, p );
            }

            if( contained ){
                retlist->push_back(sons[q]);
            }else if( clipped != NULL ){
                list<Node *> *l = (list<Node *> *)sons[q]->searchInterior(clipped);
                if( l != NULL ){
                    retlist->insert( retlist->end(), l->begin(), l->end() );
                    delete l;
                }
            }
        }
        delete clipped;
        return retlist;
    }
}

void Node::insert ( GIMSGeometry *geom ) {
    depth++;
    //printf("--------------------\n");
    //printf("   node depth: %d\n", depth++);
    //printf("   square:\n");
    //printf("      minx: %lf\n", this->square->lowerLeft->x);
    //printf("      maxx: %lf\n", this->square->upperRight->x);
    //printf("      miny: %lf\n", this->square->lowerLeft->y);
    //printf("      maxy: %lf\n", this->square->upperRight->y);
    //printf("--------------------\n");

    if( depth > 30 ){
        printf("max depth reached\n");
        printf("%lf\n", this->square->upperRight->x - this->square->lowerLeft->x );
        depth--;
        return;
    }

    GIMSGeometryList *clipped = (GIMSGeometryList *)(geom->clipToBox ( this->square ));
    
    if (clipped == NULL) {
        //printf("node not intersected\n");
        depth--;
        return;
    }
    
    if ( this->type != GRAY ) {
        /*node type is only gray when it's not a leaf*/
        
        /*merge the two lists*/
        if (this->dictionary != NULL) {
            clipped->list->insert (clipped->list->end(), this->dictionary->begin(), this->dictionary->end() );
            delete this->dictionary;
            this->dictionary = NULL;
        }
        
        if ( this->validateGeometry ( clipped ) ) {
            //printf("inserting geometry in node\n");
            this->type = BLACK;
            this->dictionary = clipped->list;
            depth--;
            return;
        } else {
            this->split();
        }
    }
    
    for (Quadrant q : quadrantList) {
        this->sons[q]->insert ( clipped );
    }
    depth--;
    delete clipped;
}

/* Returns true if the given geometry is a valid one for the calling node
   !Note! The bounding box geometry is not supported !Note!
   The behaviour is undefined in such a situation. */
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

PMQuadTree::~PMQuadTree () {

}



/*Functions that take care of the construction and maintenance of the structure*/
void PMQuadTree::build  (GIMSGeometry *geom){

}

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
    // cr->move_to( -100,-100);
    // cr->line_to( 600, 500);
    // cr->stroke();

    renderer->setScale( 400.0/this->root->square->xlength(),
                        -400.0/this->root->square->ylength() );
    renderer->setTranslation( -this->root->square->lowerLeft->x,
                              -this->root->square->upperRight->y );
    this->renderTree ( cr, this->root );
    printf("rendered the tree\n");

    if(query != NULL){
        list<Node *> *results = (list<Node *> *)(this->root->searchInterior((GIMSPolygon *)(((GIMSGeometryList *)(this->query))->list->front())));
        cr->set_source_rgba(0.0, 0.19, 0.69, 0.2);
        for( list<Node *>::iterator i = results->begin(); i!= results->end(); i++ ){
            renderer->renderFilledBBox( cr, (*i)->square );
            cr->fill();
        }
    }else{
        printf("null query\n");
    }
}

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
