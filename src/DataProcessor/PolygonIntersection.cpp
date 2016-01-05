#include "PolygonIntersection.hpp"

GIMS_Point *__ptlist_cmp__reference__;

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

#ifdef DEBUG
    printf("==== POLYGON ====\n");
#endif

    GIMS_Polygon *clippedPolygon = new GIMS_Polygon();

    for(facelist::iterator it = planargraph.faces.begin(); it != planargraph.faces.end(); it++){
        face *f = *it;

        //if the face is covered by both the polygon and the domain
        if( f->data & 8 && f->data & 2 ){
#ifdef DEBUG
            printf("---- face ----\n");
#endif
            halfedge *e   = f->boundary;
            halfedge *aux = e->next;

            GIMS_LineString *ring = new GIMS_LineString();
            ring->appendPoint( e->tail->pt );
            e->tail->pt->id = e->data & 2 ? 0 : 1;

#ifdef DEBUG
            printf("%lf %lf (%d), ", e->tail->pt->x, e->tail->pt->y, e->tail->pt->id);
#endif

            while(aux != e){
                aux->tail->pt->id = aux->data & 2 ? 0 : 1;
#ifdef DEBUG
                printf("%lf %lf (%d), ", aux->tail->pt->x, aux->tail->pt->y, aux->tail->pt->id);
#endif
                ring->appendPoint( aux->tail->pt );
                aux = aux->next;
            }

            GIMS_Point *lclone = new GIMS_Point(e->tail->pt->x, e->tail->pt->y);
            ring->appendPoint( lclone );
            clippedPolygon->appendExternalRing(ring);
#ifdef DEBUG
            printf("\n");
#endif
        }
    }

    return clippedPolygon;
}

void insertPolygonVertexes(DCEL &dcel, GIMS_Polygon *p, int id){
    for(int i=0; i<p->externalRing->size; i++){
        for(int j=0; j<p->externalRing->list[i]->size; j++){
            vertex *v = new vertex();
            v->pt = p->externalRing->list[i]->list[j];
            v->data = id;

            vertex *it = dcel.findVertex(v);
            if( it != NULL ){
                it->data |= id;
                delete v;
            }else{
                dcel.addVertex(v);
            }
        }
    }
}

void insertVertexesFromList(DCEL &dcel, list<GIMS_Point *> &ptlist, int id){
    for(list<GIMS_Point *>::iterator i = ptlist.begin(); i != ptlist.end(); i++){
        vertex *v = new vertex();
        v->pt = *i;
        v->data = id;

        vertex *it = dcel.findVertex(v);
        if( it != NULL ){
            it->data |= id;
            delete v;
        }else{
            dcel.addVertex(v);
        }
    }
}

void insertPolygonHalfedges(DCEL &dcel, intersectionset &iset){
    
    int clockwise_data, counterclockwise_data;

    for(intersectionset::iterator it = iset.begin(); it != iset.end(); it++){
        GIMS_LineSegment edge = it->first;
        list<GIMS_Point *> intersectionPoints = it->second;
    
        counterclockwise_data = edge.id == 1 ? 1 : 4;
        clockwise_data = edge.id == 1 ? 2 : 8;

        list<GIMS_Point *>::iterator prev = intersectionPoints.begin();
        list<GIMS_Point *>::iterator next = list<GIMS_Point *>::iterator(prev);
        next++;

        while(next != intersectionPoints.end()){
            GIMS_Point *p1 = *prev,
                       *p2 = *next;

            vertex *it_v1 = dcel.findVertex(p1);
            vertex *it_v2 = dcel.findVertex(p2);

            halfedge *cw  = new halfedge();
            halfedge *ccw = new halfedge();

            cw->twin  = ccw;
            cw->tail  = it_v1;
            cw->data |= clockwise_data;

            ccw->twin  = cw;
            ccw->tail  = it_v2;
            ccw->data |= counterclockwise_data;

            halfedge *it1 = dcel.findHalfedge(cw);
            halfedge *it2 = dcel.findHalfedge(ccw);
            
            if( it1 != NULL ){
                it1->data |= clockwise_data;
                delete cw;
            }else{
                dcel.addHalfedge(cw);
                it_v1->incidentEdges.push_back(cw);
            }

            if( it2 != NULL ){
                it2->data |= counterclockwise_data;
                delete ccw;
            }else{
                dcel.addHalfedge(ccw);
                it_v2->incidentEdges.push_back(ccw);
            }

            prev++;
            next++;
        }
    }
}

void connectHalfedges(DCEL &dcel){
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
}

/*returns a DCEL representing a planar graph of A and B bounded to the argument domain.*/
DCEL buildPlanarGraph(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB, GIMS_BoundingBox *domain){

    DCEL planargraph;

    /*1. clip polygons to domain*/
    GIMS_Polygon *clippedA = clipPolygonInDCEL( polygonAndDomainAsPlanarGraph(polygonA, domain) ),
                 *clippedB = clipPolygonInDCEL( polygonAndDomainAsPlanarGraph(polygonB, domain) );

    /*2. create a PSLG of the two polygons*/
    
    //2.1 find the intersections points of every edge of each polygon
    intersectionset iset = findIntersections(clippedA, clippedB);

#ifdef DEBUG
    for(intersectionset::iterator it = iset.begin(); it != iset.end(); it++){
        printf("%lf %lf --- %lf %lf\n", it->first.p1->x, it->first.p1->y, it->first.p2->x, it->first.p2->y);
        for(list<GIMS_Point *>::iterator pt = it->second.begin(); pt != it->second.end(); pt++)
            printf("%lf %lf\n", (*pt)->x, (*pt)->y);
    }
#endif

    //2.2 for every polygon vertex and intersection point create a vertex in the DCEL
    for(intersectionset::iterator it = iset.begin(); it != iset.end(); it++)
        insertVertexesFromList(planargraph, it->second, it->first.id);

    //2.3 for every edge create two halfedges
    insertPolygonHalfedges(planargraph, iset);

    //2.4 calculate the connectivity of the halfedges
    connectHalfedges(planargraph);

    //2.5 calculate the faces
    planargraph.calculateFaces();

    for( facelist::iterator it = planargraph.faces.begin(); it != planargraph.faces.end(); it++ ){

        halfedge *he = (*it)->boundary;
        halfedge *aux = he->next;

        printf("=== FACE ===\n");

        printf("%lf %lf\n", he->tail->pt->x, he->tail->pt->y);
        while(aux != he){
            printf("%lf %lf\n", aux->tail->pt->x, aux->tail->pt->y);
            aux = aux->next;
        }

    }

    return planargraph;
}

void DE9IM_pol_pol(DE9IM *resultset, GIMS_Polygon *A, GIMS_Polygon *B, GIMS_BoundingBox *domain){
    
    /*
    Rule set:
        II: if there is at least one common face then II=2, else NULL
        IB: if there is at least one edge of polygon B separating two faces enclosed by Polygon A then IB=1, else NULL
        IE: if there is at least one face uniquely enclosed by polygon A then IE=2, else NULL
        BI: if there is at least one edge of polygon A separating two faces enclosed by polygon B then 1, else NULL
        BB: if there are common edges then BB=1, else if there are common nodes BB=0, else NULL
        BE: if there is at least one face uniquely enclosed by polygon A then BE=1, else NULL
        EI: if there is at least one face uniquely enclosed by polygon B then EI=2, else NULL
        EB: if there is at least one face uniquely enclosed by polygon B then EI=1, else NULL
        EE: EE=2
    */

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

bool __ptlist_cmp__stdlist__(GIMS_Point *a, GIMS_Point *b){
    double d1 = distanceSquared2p(a, __ptlist_cmp__reference__),
           d2 = distanceSquared2p(b, __ptlist_cmp__reference__);
    return d1 < d2;
}

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
            ptlist.nodes[cur_ind] = ptlist.nodes[i];
            cur_ind++;
        }else{
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

bool event_cmp(PolygonIntersectionEvent a, PolygonIntersectionEvent b){
    if(a.pt->x < b.pt->x - ERR_MARGIN ){
        return false;
    }else if(a.pt->x < b.pt->x + ERR_MARGIN && a.pt->x > b.pt->x - ERR_MARGIN ){

        if( a.pt->y < b.pt->y - ERR_MARGIN )
            return false;
        else if( a.pt->y < b.pt->y + ERR_MARGIN && a.pt->y > b.pt->y - ERR_MARGIN ){
            
            //if x and y are equal, differentiate by PolygonIntersectionevent type
            if( a.type < b.type )
                return false;
            else if( b.type < a.type )
                return true;

            //if both x and y are equal and the PolygonIntersectionevent type is equal
            //differentiate by the given ID
            else if( a.ls.id < b.ls.id )
                return false;
            
            //finally, we must distinguish line segments that start at the same point and
            //are in the same set.
            else
                return ls_cmp(a.ls, b.ls);

        }else
            return true;
    }else 
        return true;
}

bool ls_cmp(const GIMS_LineSegment &a, const GIMS_LineSegment &b){

    if( a.p1->x < b.p1->x )
        return true;
    else if(a.p1->x == b.p1->x){
        if(a.p1->y < b.p1->y)
            return true;
        else if(a.p1->y > b.p1->y)
            return false;
    }else
        return false;

    //at this point we know that a.p1 equals b.p1 
    if( a.p2->x < b.p2->x )
        return true;
    else if(a.p2->x == b.p2->x){
        if(a.p2->y < b.p2->y)
            return true;
        else if(a.p2->y > b.p2->y)
            return false;
    }else
        return false;

    //if this point is reached, the line segments are equal
    return false;
}

double eventQueueFromMultiLineString( eventset &eventQueue, GIMS_MultiLineString *mls, int id){
    double max_x = -1e100;
    int count = 1;
    for(int i=0; i<mls->size; i++){
        for(int j=0; j<mls->list[i]->size-1; j++){
            PolygonIntersectionEvent a, b;
            GIMS_LineSegment ls = mls->list[i]->getLineSegment(j);
            ls.id = id;
            ls.osm_id = count++;

            a.ls = ls;
            b.ls = ls;
            a.type = 0; b.type = 1;
            
            if( ls.p1->x < ls.p2->x ){
                a.pt = ls.p1; b.pt = ls.p2;
            }else if(ls.p1->x > ls.p2->x){
                a.pt = ls.p2; b.pt = ls.p1;
            }else{
                if( ls.p1->y < ls.p2->y ){
                    a.pt = ls.p1; b.pt = ls.p2;
                }else{
                    a.pt = ls.p2; b.pt = ls.p1;
                }
            }

            if( ls.p1->x > max_x )
                max_x = ls.p1->x;
            if( ls.p2->x > max_x )
                max_x = ls.p2->x;

            eventQueue.push(a);
            eventQueue.push(b);
        }
    }
    return max_x;
}

GIMS_Point *getRightMostPoint(GIMS_LineSegment &ls){
    if(ls.p1->x < ls.p2->x)
        return ls.p2;
    else if( ls.p1->x > ls.p2->x )
        return ls.p1;
    else{
        if(ls.p1->y < ls.p2->y)
            return ls.p2;
        else if( ls.p1->y > ls.p2->y )
            return ls.p1;
    }
    return 0;
}

bool compareByRightmostPoint(GIMS_LineSegment &a, GIMS_LineSegment &b){
    GIMS_Point *arp = getRightMostPoint(a),
               *brp = getRightMostPoint(b);

    if(arp->x < brp->x)
        return true;
    else if( arp->x > brp->x )
        return false;
    else{
        if(arp->y < brp->y)
            return true;
        else if( arp->y > brp->y )
            return false;
    }
    return false;
}

void insertToActiveSet(lsset &l, GIMS_LineSegment &ls){
    for(lsset::iterator it = l.begin(); it != l.end(); it++){
        if( !compareByRightmostPoint(*it, ls) ){
            l.insert(it, ls);
            return;
        }
    }
    l.push_back(ls);
}

void addIntersection(intersectionset &iset, GIMS_LineSegment &A, GIMS_LineSegment &B, GIMS_Geometry *intersection){
    if( intersection == NULL )
        return;

    if( intersection->type == POINT ){
        iset[A].push_back((GIMS_Point *)intersection);
        iset[B].push_back((GIMS_Point *)intersection);
    }else{
        iset[A].push_back(((GIMS_LineSegment *)intersection)->p1);
        iset[A].push_back(((GIMS_LineSegment *)intersection)->p2);
        iset[B].push_back(((GIMS_LineSegment *)intersection)->p1);
        iset[B].push_back(((GIMS_LineSegment *)intersection)->p2);
    }   
}

void sortIntersections(intersectionset &iset){
    for(intersectionset::iterator it = iset.begin(); it != iset.end(); it++){
        __ptlist_cmp__reference__ = it->first.p1;
        it->second.sort(__ptlist_cmp__stdlist__);

        list<GIMS_Point *>::iterator prev = it->second.begin();
        list<GIMS_Point *>::iterator next = list<GIMS_Point *>::iterator(prev);
        next++;

        while(next != it->second.end()){
            if( (*next)->equals(*prev) ){
                it->second.erase(next);
                next = list<GIMS_Point *>::iterator(prev);
                next++;
            }else{
                prev++;
                next++;
            }
        }
    }
}

intersectionset findIntersections(GIMS_Polygon *polygonA, GIMS_Polygon *polygonB){

    GIMS_MultiLineString *A = polygonA->externalRing,
                         *B = polygonB->externalRing;

    intersectionset intersections(&ls_cmp);
    eventset eventQueue(&event_cmp);

    double last_x_A = eventQueueFromMultiLineString(eventQueue, A, 1);
    double last_x_B = eventQueueFromMultiLineString(eventQueue, B, 2);
    double last_x = MIN(last_x_A, last_x_B); //after this value of X, no intersections happen.
    
    lsset red, blue;

    PolygonIntersectionEvent event;
    while( !eventQueue.empty() ){
        
        event = eventQueue.top();
        eventQueue.pop();
        if( event.pt->x > last_x )
            break;

        if( event.type == 0 ){
            if( event.ls.id == 1 ){ //red
                for(lsset::iterator it = blue.begin(); it != blue.end(); it++){
                    GIMS_Geometry *g = event.ls.intersects( (&(*it)) );
                    addIntersection(intersections, event.ls, *it, g);
                }
                insertToActiveSet(red, event.ls);
            }else{ //blue
                for(lsset::iterator it = red.begin(); it != red.end(); it++){
                    GIMS_Geometry *g = event.ls.intersects( (&(*it)) );
                    addIntersection(intersections, event.ls, *it, g);
                }
                insertToActiveSet(blue, event.ls);
            }
        }

        else if( event.type == 1 ){

            intersections[event.ls].push_back( event.ls.p1 );
            intersections[event.ls].push_back( event.ls.p2 );

            if( event.ls.id == 1 ){ //red
                for(lsset::iterator it = red.begin(); it != red.end(); it++){
                    if( ls_cmp( *it, event.ls) == 0 ){
                        red.erase(it);
                        break;
                    }
                }
            }else{ //blue
                for(lsset::iterator it = blue.begin(); it != blue.end(); it++){
                    if( ls_cmp( *it, event.ls) == 0 ){
                        blue.erase(it);
                        break;
                    }
                }
            }
            //event.ls.deepDelete();
        }
    }

    sortIntersections(intersections);
    return intersections;
}



