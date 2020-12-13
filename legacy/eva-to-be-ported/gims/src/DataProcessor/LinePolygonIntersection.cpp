#include "LinePolygonIntersection.hpp"

GIMS_MultiLineString *clipMultiLineStringInDCEL(DCEL &planargraph, GIMS_BoundingBox *domain)
{
        GIMS_MultiLineString *clippedMLS = new GIMS_MultiLineString();

        for (halfedgelist::iterator it = planargraph.halfedges.begin(); it != planargraph.halfedges.end(); it++) {
                halfedge *he = *it;

                if (!(he->data & (1 | 2)))
                        continue;

                GIMS_Point *p1 = he->tail->pt, *p2 = he->twin->tail->pt;

                GIMS_Point midpoint = GIMS_Point(p1->x + (p2->x - p1->x) / 2.0, p1->y + (p2->y - p1->y) / 2.0);

                bool keepHalfedge = midpoint.isInsideBox(domain);

                if (keepHalfedge) {
                        GIMS_LineString *ls = new GIMS_LineString(2);
                        ls->appendPoint(p1);
                        ls->appendPoint(p2);
                        clippedMLS->append(ls);
                }
        }

        return clippedMLS;
}

DCEL buildPlanarGraph_mlspol(GIMS_MultiLineString *mls, GIMS_Polygon *pol, GIMS_BoundingBox *domain)
{
        DCEL planargraph;

        //clip the linestring
        GIMS_Polygon *mls_container = new GIMS_Polygon(mls, NULL);
        DCEL mls_pslg = polygonAndDomainAsPlanarGraph(mls_container, domain);
        mls_container->externalRing = NULL;
        delete mls_container;
        GIMS_MultiLineString *mls_clipped = clipMultiLineStringInDCEL(mls_pslg, domain);

        //clip the polygon
        GIMS_Polygon *clippedPol = clipPolygonInDCEL(polygonAndDomainAsPlanarGraph(pol, domain));

        //find nodes of the DCEL
        intersectionset iset = findIntersections_mlspol(mls_clipped, clippedPol);
        deleteAllExceptPoints(mls_clipped);
        deleteAllExceptPoints(clippedPol);

        //2.2 for every vertex and intersection point create a vertex in the DCEL
        for (intersectionset::iterator it = iset.begin(); it != iset.end(); it++)
                insertVertexesFromList(planargraph, it->second, it->first.id);

#ifdef DEBUG_LS_POL_INT
        cout << "vertexes" << endl;
        for (vertexlist::iterator it = planargraph.vertexes.begin(); it != planargraph.vertexes.end(); it++) {
                cout << (*it)->pt->toWkt() << " (";
                printData((*it)->data);
                cout << ")" << endl;
        }
        cout << endl;
#endif

        //2.3 for every edge create two halfedges
        insertPolygonHalfedges(planargraph, iset);

#ifdef DEBUG_LS_POL_INT
        cout << "vertexes" << endl;
        for (halfedgelist::iterator it = planargraph.halfedges.begin(); it != planargraph.halfedges.end(); it++) {
                cout << (*it)->tail->pt->toWkt() << " --- " << (*it)->twin->tail->pt->toWkt() << " (";
                printData((*it)->data);
                cout << ")" << endl;
        }
        cout << endl;
#endif

        //2.4 connect the halfedges
        connectHalfedges(planargraph);

        //2.5 calculate the faces
        planargraph.calculateFaces();

        //2.6 calculate face data
        calculateFaceData_mlspol(planargraph, mls, pol, domain);

        return planargraph;
}

DCEL buildPlanarGraph_polmls(GIMS_Polygon *pol, GIMS_MultiLineString *mls, GIMS_BoundingBox *domain)
{
        DCEL planargraph;

        //clip the linestring
        GIMS_Polygon *mls_container = new GIMS_Polygon(mls, NULL);
        DCEL mls_pslg = polygonAndDomainAsPlanarGraph(mls_container, domain);
        mls_container->externalRing = NULL;
        delete mls_container;
        GIMS_MultiLineString *mls_clipped = clipMultiLineStringInDCEL(mls_pslg, domain);

        //clip the polygon
        GIMS_Polygon *clippedPol = clipPolygonInDCEL(polygonAndDomainAsPlanarGraph(pol, domain));

        //find nodes of the DCEL
        intersectionset iset = findIntersections_polmls(clippedPol, mls_clipped);
        deleteAllExceptPoints(mls_clipped);
        deleteAllExceptPoints(clippedPol);

        //2.2 for every vertex and intersection point create a vertex in the DCEL
        for (intersectionset::iterator it = iset.begin(); it != iset.end(); it++)
                insertVertexesFromList(planargraph, it->second, it->first.id);

        //2.3 for every edge create two halfedges
        insertPolygonHalfedges(planargraph, iset);

        //2.4 connect the halfedges
        connectHalfedges(planargraph);

        //2.5 calculate the faces
        planargraph.calculateFaces();

        //2.6 calculate face data
        calculateFaceData_polmls(planargraph, pol, mls, domain);

        return planargraph;
}

void DE9IM_mls_pol(DE9IM *resultset, GIMS_MultiLineString *query, GIMS_Polygon *other, GIMS_BoundingBox *domain)
{
        bool disjoint = true;
        for (int i = 0; disjoint && i < query->size; i++) {
                if (!(other->approximation->isDisjoint(&(query->list[i]->bbox)))) {
                        disjoint = false;
                }
        }

        if (disjoint) {
                matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);
                resultset->setIE(matrix_index, 1);
                resultset->setBE(matrix_index, 0);
                resultset->setEI(matrix_index, 2);
                resultset->setEB(matrix_index, 1);
                return;
        }

        DCEL planargraph = buildPlanarGraph_mlspol(query, other, domain);

        /* With the calculated planar graph, it is now possible to draw conclusions about the
     * intersection matrix of the polygons. */
        bool hasPointIntersection = false, hasCommonEdges = false, linestringCrossesPolygon = false,
             polygonHasUniqueEdge = false, linestringHasOutsidePoint = false;

        for (vertexlist::iterator it = planargraph.vertexes.begin(); it != planargraph.vertexes.end(); it++) {
                vertex *v = *it;
                if (isVertexOfA(v) && isVertexOfB(v)) {
                        hasPointIntersection = true;
                }
        }

        for (halfedgelist::iterator it = planargraph.halfedges.begin(); it != planargraph.halfedges.end(); it++) {
                halfedge *he = *it;

                if (isEdgeOfA(he) && isExteriorEdgeOfB(he))
                        hasCommonEdges = true;

                if (isEdgeOfA(he) && isFaceOfB(he->left) && isFaceOfB(he->twin->left))
                        linestringCrossesPolygon = true;

                if (isExteriorEdgeOfB(he) && !isEdgeOfA(he))
                        polygonHasUniqueEdge = true;

                if (isEdgeOfA(he) && !isFaceOfB(he->left) && !isFaceOfB(he->twin->left))
                        linestringHasOutsidePoint = true;
        }

        /*
    printf("hasPointIntersection     : %s\n", hasPointIntersection      ? "true" : "false");
    printf("hasCommonEdges           : %s\n", hasCommonEdges            ? "true" : "false");
    printf("linestringCrossesPolygon : %s\n", linestringCrossesPolygon  ? "true" : "false");
    printf("polygonHasUniqueEdge     : %s\n", polygonHasUniqueEdge      ? "true" : "false");
    printf("linestringHasOutsidePoint: %s\n", linestringHasOutsidePoint ? "true" : "false");
*/

        matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);

        if (linestringCrossesPolygon) {
                resultset->setII(matrix_index, 1);
                resultset->setIntersect(matrix_index, 1);
        }
        if (linestringHasOutsidePoint) {
                resultset->setIE(matrix_index, 1);
                resultset->setBE(matrix_index, 0);
        }

        if (polygonHasUniqueEdge)
                resultset->setEB(matrix_index, 1);

        resultset->setEI(matrix_index, 2);

        if (hasCommonEdges)
                resultset->setIntersect(matrix_index, 1);

        if (hasPointIntersection)
                resultset->setIntersect(matrix_index, 0);

        clearSearchByproducts();
}

void DE9IM_pol_mls(DE9IM *resultset, GIMS_Polygon *query, GIMS_MultiLineString *other, GIMS_BoundingBox *domain)
{
        bool disjoint = true;
        for (int i = 0; disjoint && i < other->size; i++) {
                if (!(query->approximation->isDisjoint(&(other->list[i]->bbox)))) {
                        disjoint = false;
                }
        }

        if (disjoint) {
                matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);
                resultset->setEI(matrix_index, 1);
                resultset->setEB(matrix_index, 0);
                resultset->setIE(matrix_index, 2);
                resultset->setBE(matrix_index, 1);
                return;
        }

        DCEL planargraph = buildPlanarGraph_polmls(query, other, domain);

        /* With the calculated planar graph, it is now possible to draw conclusions about the
     * intersection matrix of the polygons. */
        bool hasPointIntersection = false, hasCommonEdges = false, linestringCrossesPolygon = false,
             polygonHasUniqueEdge = false, linestringHasOutsidePoint = false;

        for (vertexlist::iterator it = planargraph.vertexes.begin(); it != planargraph.vertexes.end(); it++) {
                vertex *v = *it;
                if (isVertexOfA(v) && isVertexOfB(v)) {
                        hasPointIntersection = true;
                }
        }

        for (halfedgelist::iterator it = planargraph.halfedges.begin(); it != planargraph.halfedges.end(); it++) {
                halfedge *he = *it;

                if (isEdgeOfB(he) && isExteriorEdgeOfA(he))
                        hasCommonEdges = true;

                if (isEdgeOfB(he) && isFaceOfA(he->left) && isFaceOfA(he->twin->left))
                        linestringCrossesPolygon = true;

                if (isExteriorEdgeOfA(he) && !isEdgeOfB(he))
                        polygonHasUniqueEdge = true;

                if (isEdgeOfB(he) && !isFaceOfA(he->left) && !isFaceOfA(he->twin->left))
                        linestringHasOutsidePoint = true;
        }

        /*
    printf("hasPointIntersection     : %s\n", hasPointIntersection      ? "true" : "false");
    printf("hasCommonEdges           : %s\n", hasCommonEdges            ? "true" : "false");
    printf("linestringCrossesPolygon : %s\n", linestringCrossesPolygon  ? "true" : "false");
    printf("polygonHasUniqueEdge     : %s\n", polygonHasUniqueEdge      ? "true" : "false");
    printf("linestringHasOutsidePoint: %s\n", linestringHasOutsidePoint ? "true" : "false");
*/

        matrix_t::iterator matrix_index = resultset->getMatrixIndex(other->id);

        if (linestringCrossesPolygon) {
                resultset->setII(matrix_index, 1);
                resultset->setIntersect(matrix_index, 1);
        }
        if (linestringHasOutsidePoint) {
                resultset->setEI(matrix_index, 1);
                resultset->setEB(matrix_index, 0);
        }

        if (polygonHasUniqueEdge)
                resultset->setBE(matrix_index, 1);

        resultset->setIE(matrix_index, 2);

        if (hasCommonEdges)
                resultset->setIntersect(matrix_index, 1);

        if (hasPointIntersection)
                resultset->setIntersect(matrix_index, 0);

        clearSearchByproducts();
}

/*The art of good coding .... xD two functions with the exact same code... !!!*/
/*If anyone is reading this.... I'm sorry, but I'm in a hurry and this function has 
  already been somewhat tested because there's another copy of it in the polygon 
  intersection... ahaha that's right, two copies in the same file and it is still
  elsewhere too. Anyone with intents of cleaning up this mess is crying right now.*/
intersectionset findIntersections_mlspol(GIMS_MultiLineString *mls, GIMS_Polygon *pol)
{
        GIMS_MultiLineString *A = mls, *B = pol->externalRing;

        intersectionset intersections(&ls_cmp);
        eventset eventQueue(&event_cmp);

        addEndpointsToIntersectionSet(intersections, A, 1);
        addEndpointsToIntersectionSet(intersections, B, 2);

        double last_x_A = eventQueueFromMultiLineString(eventQueue, A, 1);
        double last_x_B = eventQueueFromMultiLineString(eventQueue, B, 2);
        double last_x = MIN(last_x_A, last_x_B); //after this value of X, no intersections happen.

        PolygonIntersectionEvent event;
        lsset red, blue;

        while (!eventQueue.empty()) {
                event = eventQueue.top();
                eventQueue.pop();
                if (event.pt->x > last_x)
                        break;

                if (event.type == 0) {
                        if (event.ls.id == 1) { //red
                                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                insertToActiveSet(red, event.ls);
                        } else { //blue
                                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                insertToActiveSet(blue, event.ls);
                        }
                }

                else if (event.type == 1) {
                        if (event.ls.id == 1) { //red
                                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                                        if (ls_cmp(*it, event.ls) == 0) {
                                                red.erase(it);
                                                break;
                                        }
                                }
                        } else { //blue
                                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                                        if (ls_cmp(*it, event.ls) == 0) {
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

intersectionset findIntersections_polmls(GIMS_Polygon *pol, GIMS_MultiLineString *mls)
{
        GIMS_MultiLineString *A = pol->externalRing, *B = mls;

        intersectionset intersections(&ls_cmp);
        eventset eventQueue(&event_cmp);

        addEndpointsToIntersectionSet(intersections, A, 1);
        addEndpointsToIntersectionSet(intersections, B, 2);

        double last_x_A = eventQueueFromMultiLineString(eventQueue, A, 1);
        double last_x_B = eventQueueFromMultiLineString(eventQueue, B, 2);
        double last_x = MIN(last_x_A, last_x_B); //after this value of X, no intersections happen.

        lsset red, blue;

        PolygonIntersectionEvent event;
        while (!eventQueue.empty()) {
                event = eventQueue.top();
                eventQueue.pop();
                if (event.pt->x > last_x)
                        break;

                if (event.type == 0) {
                        if (event.ls.id == 1) { //red
                                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                insertToActiveSet(red, event.ls);
                        } else { //blue
                                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                //we need this additional step because there may be self intersections in the MLS
                                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                                        GIMS_Geometry *g = event.ls.intersects((&(*it)));
                                        addIntersection(intersections, event.ls, *it, g);
                                }
                                insertToActiveSet(blue, event.ls);
                        }
                }

                else if (event.type == 1) {
                        if (event.ls.id == 1) { //red
                                for (lsset::iterator it = red.begin(); it != red.end(); it++) {
                                        if (ls_cmp(*it, event.ls) == 0) {
                                                red.erase(it);
                                                break;
                                        }
                                }
                        } else { //blue
                                for (lsset::iterator it = blue.begin(); it != blue.end(); it++) {
                                        if (ls_cmp(*it, event.ls) == 0) {
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

void calculateFaceData_polmls(DCEL &dcel, GIMS_Polygon *pol, GIMS_MultiLineString *mls, GIMS_BoundingBox *domain)
{
        for (facelist::iterator it = dcel.faces.begin(); it != dcel.faces.end(); it++) {
                face *f = *it;

                halfedge *e = f->boundary;
                halfedge *aux = e->next;

                /* associate the data of the bounding edges with the face's data*/
                f->data |= e->data;
                while (aux != e) {
                        f->data |= aux->data;
                        aux = aux->next;
                }

                /* if at this point we don't have information about the face's intersection with either
         * the polygon or the domain, further processing has to be done to determine it.*/
                if (!(f->data & (2 + 1))) {
                        aux = aux->next;
                        while (aux != e) {
                                double x = aux->tail->pt->x + (aux->twin->tail->pt->x - aux->tail->pt->x) / 2.0,
                                       y = aux->tail->pt->y + (aux->twin->tail->pt->y - aux->tail->pt->y) / 2.0;
                                GIMS_Point middlepoint = GIMS_Point(x, y);

                                if (pol->containsPointWithinDomain(&middlepoint, domain)) {
                                        f->data |= 2;
                                        break;
                                }
                                aux = aux->next;
                        }

                        /*if not inside, it's outside.*/
                        if (!(f->data & 2))
                                f->data |= 1;
                }
        }
}

void calculateFaceData_mlspol(DCEL &dcel, GIMS_MultiLineString *mls, GIMS_Polygon *pol, GIMS_BoundingBox *domain)
{
        for (facelist::iterator it = dcel.faces.begin(); it != dcel.faces.end(); it++) {
                face *f = *it;

                halfedge *e = f->boundary;
                halfedge *aux = e->next;

                /* associate the data of the bounding edges with the face's data*/
                f->data |= e->data;
                while (aux != e) {
                        f->data |= aux->data;
                        aux = aux->next;
                }

                /* if at this point we don't have information about the face's intersection with either
         * the polygon or the domain, further processing has to be done to determine it.*/
                if (!(f->data & (8 + 4))) {
                        aux = aux->next;
                        while (aux != e) {
                                double x = aux->tail->pt->x + (aux->twin->tail->pt->x - aux->tail->pt->x) / 2.0,
                                       y = aux->tail->pt->y + (aux->twin->tail->pt->y - aux->tail->pt->y) / 2.0;
                                GIMS_Point middlepoint = GIMS_Point(x, y);

                                if (pol->containsPointWithinDomain(&middlepoint, domain)) {
                                        f->data |= 8;
                                        break;
                                }
                                aux = aux->next;
                        }

                        /*if not inside, it's outside.*/
                        if (!(f->data & 8))
                                f->data |= 4;
                }
        }
}
