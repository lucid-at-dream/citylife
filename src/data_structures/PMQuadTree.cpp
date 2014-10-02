#include "PMQuadTree.hpp"

//#define DEBUG

quadrant quadrantList[] = {NW, NE, SE, SW};
char quadrantLabels[][3] = {"NW", "NE", "SE", "SW"};
double xmultiplier[] = {0.0, 0.5, 0.5, 0.0};
double ymultiplier[] = {0.0, 0.0, -0.5, -0.5};


/***
 *    ##.....##.########.########..########...#######...######..########.
 *    ##.....##.##.......##.....##.##.....##.##.....##.##....##.##.......
 *    ##.....##.##.......##.....##.##.....##.##.....##.##.......##.......
 *    ##.....##.######...########..########..##.....##..######..######...
 *    .##...##..##.......##...##...##.....##.##.....##.......##.##.......
 *    ..##.##...##.......##....##..##.....##.##.....##.##....##.##.......
 *    ...###....########.##.....##.########...#######...######..########.
 */
void dumpTreeJSON (node *R) {
    /*if it is a leaf node*/
    if (R->type != GRAY) {
        dumpLeafJSON (R);
        
    } else {
        printf ("{");
        printf ("'type': 'GRAY',\n");
        printf ("'square': {'size': %lf, 'upperLeft': {'x': %lf, 'y': %lf}},\n",
                R->s->len, R->s->leftUpperCorner->x, R->s->leftUpperCorner->y);
        printf ("'sons':{");
        
        for (quadrant q : quadrantList) {
            printf ("'%s' : ", quadrantLabels[q]);
            dumpTreeJSON (R->son[q]);
            printf (",");
        }
        
        printf ("}}");
    }
}

void dumpLeafJSON (node *R) {
    printf ("{");
    
    if (R->type == WHITE) {
        printf ("'type': 'WHITE',");
        printf ("'square': {'size': %lf, 'upperLeft': {'x': %lf, 'y': %lf}},\n",
                R->s->len, R->s->leftUpperCorner->x, R->s->leftUpperCorner->y);
        printf ("}");
        return;
    }
    
    printf ("'type': 'BLACK',\n");
    printf ("'square': {'size': %lf, 'upperLeft': {'x': %lf, 'y': %lf}},\n",
            R->s->len, R->s->leftUpperCorner->x, R->s->leftUpperCorner->y);
            
    printf ("'edges': [");
    
    for ( std::list<edgelistNode *>::iterator e = R->dictionary->begin();
            e != R->dictionary->end(); e++ ) {
        if ( (*e)->datatype == EDGE ) {
            edge *trimmed = trimEdge ( (*e)->data->e, R->s);/**/
            /*edge *trimmed = (*e)->data->e;/**/
            printf ("{'color':'%s', 'p1':{'x': %lf, 'y': %lf},'p2':{'x': %lf, 'y': %lf}},\n",
                    (*e)->color,
                    trimmed->p1->x, trimmed->p1->y,
                    trimmed->p2->x, trimmed->p2->y);
            free (trimmed->p1); free (trimmed->p2); free (trimmed);
        }
    }
    
    printf ("],\n");
    
    printf ("'points': [");
    
    for ( std::list<edgelistNode *>::iterator e = R->dictionary->begin();
            e != R->dictionary->end(); e++ ) {
        if ( (*e)->datatype == POINT ) {
            printf ("{'x': %lf, 'y': %lf},",
                    (*e)->data->pt->x, (*e)->data->pt->y );
        }
    }
    
    printf ("]}");
}

edge *trimEdge (edge *e, square *s) {

    bool p1inside = ptInsideSquare (e->p1, s),
         p2inside = ptInsideSquare (e->p2, s);
         
    /*if both endpoints lie inside the square*/
    if ( p1inside && p2inside ) {
        return createEdge (clonePoint (e->p1), clonePoint (e->p2) );
    }
    
    /*these are the square limits*/
    double ymax = s->leftUpperCorner->y,
           ymin = s->leftUpperCorner->y - s->len,
           xmax = s->leftUpperCorner->x + s->len,
           xmin = s->leftUpperCorner->x;
           
    /*if line is vertical*/
    if ( e->p1->x == e->p2->x ) {
        if ( p1inside ) {
            double int_y = e->p1->y > e->p2->y ? ymin : ymax;
            return createEdge ( clonePoint (e->p1), createPoint ( e->p1->x, int_y ) );
            
        } else if ( p2inside ) {
            double int_y = e->p2->y > e->p1->y ? ymin : ymax;
            return createEdge ( createPoint ( e->p1->x, int_y ), clonePoint (e->p2) );
            
        } else {
            return createEdge ( createPoint ( e->p1->x, ymin ),
                                createPoint ( e->p1->x, ymax ) );
        }
        
        /*if line is horizontal*/
        
    } else if ( e->p1->y == e->p2->y ) {
        if ( p1inside ) {
            double int_x = e->p1->x > e->p2->x ? xmin : xmax;
            return createEdge ( clonePoint (e->p1), createPoint ( int_x, e->p1->y ) );
            
        } else if ( p2inside ) {
            double int_x = e->p2->x > e->p1->x ? xmin : xmax;
            return createEdge ( createPoint ( int_x, e->p1->y ), clonePoint (e->p2) );
            
        } else {
            return createEdge ( createPoint ( xmin, e->p1->y ),
                                createPoint ( xmax, e->p1->y ) );
        }
    }
    
    /*calculate the intersection points and alloc corresponding points*/
    double m = (e->p1->y - e->p2->y) / (e->p1->x - e->p2->x);
    double b = e->p1->y - m * e->p1->x;
    
    double int_ymax = (ymax - b) / m,
           int_ymin = (ymin - b) / m,
           int_xmax = m * xmax + b,
           int_xmin = m * xmin + b;
           
    point *top_pt    = createPoint ( int_ymax, ymax ),
           *bottom_pt = createPoint ( int_ymin, ymin ),
            *left_pt   = createPoint ( xmin, int_xmin ),
             *right_pt  = createPoint ( xmax, int_xmax );
             
    /*For each intersection point we check if it is within the square. also, to
      prevent double counting intersection points that lie in the square's
      corner we check if a similar point hasn't been counted*/
    point *int_p1 = p1inside ? clonePoint (e->p1) : NULL;
    point *int_p2 = p2inside ? clonePoint (e->p2) : NULL;
    
    
    point *int_pts[] = {top_pt, bottom_pt, left_pt, right_pt, int_p1, int_p2};
    
    for (int i = 0; i < 4; i++) {
        if ( ! (ptInsideSquare ( int_pts[i], s )
                && lineptInsideLineSeg (int_pts[i], e) ) ) {
            free ( int_pts[i] );
            int_pts[i] = NULL;
        }
    }
    
    point *p1 = NULL, *p2 = NULL;
    
    for (int i = 0; i < 6; i++) {
        if (int_pts[i] != NULL) {
            if (p1 == NULL) {
                p1 = int_pts[i];
                
            } else {
                if ( (p1->x != int_pts[i]->x || p1->y != int_pts[i]->y ) && p2 == NULL ) {
                    p2 = int_pts[i];
                    
                } else {
                    free (int_pts[i]);
                    int_pts[i] = NULL;
                }
            }
        }
    }
    
    /*If the edge hits the square exactly on the corner, the intersection is
      a single point and therefore p2 will be set to NULL*/
    if ( p1 != NULL && p2 == NULL ) {
        return createEdge (p1, clonePoint (p1) );
        
    } else if ( p1 != NULL && p2 != NULL ) {
        return createEdge (p1, p2);
        
    } else {
        perror ("trimEdge called on a edge that did not intersect the square");
        exit (1);
    }
}

/***
 *    ####.##....##.########.########.########...######..########..######..########.####..#######..##....##.
 *    .##..###...##....##....##.......##.....##.##....##.##.......##....##....##.....##..##.....##.###...##.
 *    .##..####..##....##....##.......##.....##.##.......##.......##..........##.....##..##.....##.####..##.
 *    .##..##.##.##....##....######...########...######..######...##..........##.....##..##.....##.##.##.##.
 *    .##..##..####....##....##.......##...##.........##.##.......##..........##.....##..##.....##.##..####.
 *    .##..##...###....##....##.......##....##..##....##.##.......##....##....##.....##..##.....##.##...###.
 *    ####.##....##....##....########.##.....##..######..########..######.....##....####..#######..##....##.
 */
/*
void intersect(node *root, std::list<edgelistNode *> *query) {
*/
/*Is it possible to perform a pseudo-insertion*/
/*    std::list<node *> affectedNodes = insert(root, query);


    for( node *n: affectedNodes) {
*/        /*Find if the lines contained intersect the query*/
/*    }

}
*/


/***
 *    .######...#######..##....##..######..########.########..##.....##..######..########.####..#######..##....##.
 *    ##....##.##.....##.###...##.##....##....##....##.....##.##.....##.##....##....##.....##..##.....##.###...##.
 *    ##.......##.....##.####..##.##..........##....##.....##.##.....##.##..........##.....##..##.....##.####..##.
 *    ##.......##.....##.##.##.##..######.....##....########..##.....##.##..........##.....##..##.....##.##.##.##.
 *    ##.......##.....##.##..####.......##....##....##...##...##.....##.##..........##.....##..##.....##.##..####.
 *    ##....##.##.....##.##...###.##....##....##....##....##..##.....##.##....##....##.....##..##.....##.##...###.
 *    .######...#######..##....##..######.....##....##.....##..#######...######.....##....####..#######..##....##.
 */
void splitNode (node *R) {
    /*creates four new sons (one for each quadrant) in node R*/
    for (quadrant q : quadrantList) {
        /*create and link corresponding square*/
        point *upperLeft = createPoint (R->s->leftUpperCorner->x + R->s->len *
                                        xmultiplier[q],
                                        R->s->leftUpperCorner->y + R->s->len * ymultiplier[q]);
        square *S = createSquare ( upperLeft, R->s->len / 2.0 );
        
        /*create and append a new node*/
        node *N = createLeafNode (S);
        N->s = S;
        R->son[q] = N;
    }
    
    R->type = GRAY;
}

square *createSquare ( point *leftUpperCorner, double length ) {
    square *s = (square *) calloc (1, sizeof (square) );
    s->leftUpperCorner = leftUpperCorner;
    s->len = length;
    return s;
}

edge *createEdge (point *p1, point *p2) {
    edge *e = (edge *) calloc (1, sizeof (edge) );
    e->p1 = p1;
    e->p2 = p2;
    return e;
}

point *clonePoint ( point *p ) {
    return createPoint (p->x, p->y);
}

point *createPoint (double x, double y) {
    point *pt = (point *) calloc (1, sizeof (point) );
    pt->x = x;
    pt->y = y;
    return pt;
}

node *createLeafNode (square *s) {
    /*Creates a new leaf node marked as empty (white)*/
    node *newnode = (node *) calloc (1, sizeof (node) );
    
    for (quadrant q : quadrantList) {
        newnode->son[q] = NULL;
    }
    
    newnode->s = s;
    newnode->type = WHITE;
    newnode->dictionary = NULL;
    
    return newnode;
}

bool validateSquare (std::list<edgelistNode *> *edges, square *S) {

    for ( std::list<edgelistNode *>::iterator e = edges->begin(); e != edges->end();
            e++ ) {
        if ( (*e)->datatype == POINT ) {
            return validateVertexSharing ( (*e)->data->pt, edges, e, S );
            
        } else if ( ptInsideSquare ( (*e)->data->e->p1, S )
                    && ptInsideSquare ( (*e)->data->e->p2, S ) ) {
            /*if the square has both edge endpoints contained*/
            return false;
            
        } else if ( ptInsideSquare ( (*e)->data->e->p1, S ) ) {
            return validateVertexSharing ( (*e)->data->e->p1, edges, e, S );
            
        } else if ( ptInsideSquare ( (*e)->data->e->p2, S ) ) {
            return validateVertexSharing ( (*e)->data->e->p2, edges, e, S );
        }
    }
    
    return true;
}

/*check if all edges either contain Point P and don't have their other
vertex inside S, or don't have any of the two vertexes inside S*/
bool validateVertexSharing ( point *pt, std::list<edgelistNode *> *edges,
                             std::list<edgelistNode *>::iterator e, square *S ) {
                             
    for (e++; e != edges->end(); e++ ) {
    
        if ( (*e)->datatype == EDGE ) {
            if ( (*e)->data->e->p1->x == pt->x &&
                    (*e)->data->e->p1->y == pt->y ) {
                if ( ptInsideSquare ( (*e)->data->e->p2, S) ) {
                    return false;
                }
                
            } else if ( (*e)->data->e->p2->x == pt->x &&
                        (*e)->data->e->p2->y == pt->y ) {
                if ( ptInsideSquare ( (*e)->data->e->p1, S) ) {
                    return false;
                }
                
            } else if ( ptInsideSquare ( (*e)->data->e->p1, S)
                        || ptInsideSquare ( (*e)->data->e->p2, S) ) {
                return false;
            }
            
        } else if ( (*e)->datatype == POINT ) {
            if ( ( (*e)->data->pt->x != pt->x || (*e)->data->pt->y != pt->y) &&
                    ptInsideSquare ( (*e)->data->pt, S) ) {
                return false;
            }
        }
        
    }
    
    return true;
}

void insert (std::list<edgelistNode *> *edges, node *R) {
    std::list<edgelistNode *> *L = clipLines (edges, R->s);
    
    if (L->size() == 0) {
        delete L;
        return;
    }
    
    if ( R->type != GRAY ) {
        /*node type is only gray when it's not a leaf*/
        
        /*merge the two lists*/
        if (R->dictionary != NULL) {
            L->insert (L->end(), R->dictionary->begin(), R->dictionary->end() );
        }
        
        if ( validateSquare (L, R->s) ) {
        
#ifdef DEBUG
            printf ("#square size = %lf\n", R->s->len);
            printf ("#>%lf %lf\n", R->s->leftUpperCorner->x, R->s->leftUpperCorner->y);
            printf ("#list size = %d\n", (int) (L->size() ) );
            
            for ( std::list<edgelistNode *>::iterator e = L->begin(); e != L->end(); e++ ) {
                printf ("#>[%lf %lf],", (*e)->data->e->p1->x, (*e)->data->e->p1->y);
                printf ("#  ---  [%lf %lf]\n", (*e)->data->e->p2->x, (*e)->data->e->p2->y);
            }
            
#endif
            
            R->type = BLACK;
            
            if (R->dictionary != NULL) {
                delete R->dictionary;
            }
            
            R->dictionary = L;
            return;
            
        } else {
            splitNode (R);
        }
    }
    
    for (quadrant q : quadrantList) {
        insert (L, R->son[q]);
    }
    
    delete L;
}

std::list<edgelistNode *> *clipLines (std::list<edgelistNode *> *L, square *S) {
    std::list<edgelistNode *> *clipped = new std::list<edgelistNode *>;
    
    for ( std::list<edgelistNode *>::iterator e = L->begin(); e != L->end(); e++ ) {
        if ( clipSquare (*e, S) ) {
            clipped->push_back (*e);
        }
    }
    
    return clipped;
}

bool clipSquare (edgelistNode *e, square *S) {
    if ( e->datatype == EDGE ) {
        return edgeInsideSquare ( e->data->e, S );
        
    } else {
        return ptInsideSquare ( e->data->pt, S );
    }
}

bool lineptInsideLineSeg (point *pt, edge *e) {
    double maxx = e->p1->x > e->p2->x ? e->p1->x : e->p2->x,
           minx = e->p1->x <= e->p2->x ? e->p1->x : e->p2->x;
    return pt->x <= maxx + ERR_MARGIN && pt->x >= minx - ERR_MARGIN;
}

bool ptInsideSquare (point *pt, square *S) {

    double ymax = S->leftUpperCorner->y + ERR_MARGIN,
           ymin = S->leftUpperCorner->y - S->len - ERR_MARGIN,
           xmax = S->leftUpperCorner->x + S->len + ERR_MARGIN,
           xmin = S->leftUpperCorner->x - ERR_MARGIN;
           
    if ( pt->x <= xmax && pt->x >= xmin &&
            pt->y <= ymax && pt->y >= ymin ) {
        return true;
    }
    
    return false;
}

double ptEdgeSide (point *p, edge *e) {
    double s = (e->p2->x - e->p1->x) * (p->y - e->p1->y) -
               (e->p2->y - e->p1->y) * (p->x - e->p1->x);
    //return s < 0 ? RIGHT : s > 0 ? LEFT : ALIGNED;
    return s;
}

bool edgeInsideSquare ( edge *e, square *S ) {

    point leftUpper  = {S->leftUpperCorner->x, S->leftUpperCorner->y  },
          rightUpper = { leftUpper.x + S->len, leftUpper.y          },
          leftLower  = { leftUpper.x         , leftUpper.y - S->len },
          rightLower = { leftUpper.x + S->len, leftUpper.y - S->len };
    point *squarePoints[] = {&leftUpper, &rightUpper, &rightLower, &leftLower};
    
    /*If all square's points lie on the same side of the line defined by the
      edge's two endpoints, then there's no intersection*/
    bool allOnSameSide = true;
    double prev = ptEdgeSide (squarePoints[0], e);
    
    for ( int i = 1; i < 4; i++ ) {
        double s = ptEdgeSide (squarePoints[i], e);
        
        if ( (s > ERR_MARGIN && prev <= ERR_MARGIN) ||
                (s < -ERR_MARGIN && prev >= -ERR_MARGIN) ||
                (s <= ERR_MARGIN && s >= -ERR_MARGIN && (prev > ERR_MARGIN
                        || prev < -ERR_MARGIN ) ) ) {
            allOnSameSide = false;
            break;
        }
        
        prev = s;
    }
    
    if (allOnSameSide) {
        return false;
    }
    
    /*this part is reached only if the line defined by the two edge's endpoints
      intersects the square. Therefore, we do a projection on both the x and y
      axis of both the edge and the square and check for overlapings*/
    if (e->p1->x > rightUpper.x && e->p2->x > rightUpper.x) {
        /*edge is to the right of the rectangle*/
        return false;
    }
    
    if (e->p1->x < leftUpper.x && e->p2->x < leftUpper.x) {
        /*edge is to the left of the rectangle*/
        return false;
    }
    
    if (e->p1->y > leftUpper.y && e->p2->y > leftUpper.y) {
        /*edge is above of the rectangle*/
        return false;
    }
    
    if (e->p1->y < leftLower.y && e->p2->y < leftLower.y) {
        /*edge is above of the rectangle*/
        return false;
    }
    
    return true;
}
