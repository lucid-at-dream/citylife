#include "GIMSGeometry.hpp"
#include "PMQuadTree.hpp"
#include "DebRender.hpp"
#include "gtest/gtest.h"
#include <cmath>
#include <cstdio>
#include <list>

using namespace GIMSGEOMETRY;
using namespace PMQUADTREE;
using namespace std;

namespace {

// The fixture for testing class Foo.
class PMQuadTreeTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  PMQuadTreeTest() {
    // You can do set-up work for each test here.
  }

  virtual ~PMQuadTreeTest() {
    // You can do clean-up work that doesn't throw exceptions here.
  }

  // If the constructor and destructor are not enough for setting up
  // and cleaning up each test, you can define the following methods:

  virtual void SetUp() {
    // Code here will be called immediately after the constructor (right
    // before each test).
  }

  virtual void TearDown() {
    // Code here will be called immediately after each test (right
    // before the destructor).
  }

  // Objects declared here can be used by all tests in the test case for Foo.
};

// Tests particular cases of input to function goNorth
TEST_F(PMQuadTreeTest, goNorth) {
  GIMSPoint *lowerLeft = new GIMSPoint(0,0),
            *upperRight = new GIMSPoint(1,1);
  GIMSBoundingBox *box = new GIMSBoundingBox(lowerLeft, upperRight);

  PMQuadTree *tree = new PMQuadTree(box);
  Node *n = tree->root;
  n->split();
  n->sons[NW]->split();
  n->sons[NW]->sons[SE]->split();


  GIMSBoundingBox *s1 = n->sons[SW]->goNorth(0.4125)->square,
                  *s2 = n->sons[NW]->sons[SE]->sons[SE]->square;

  printf( "code: %lf %lf %lf %lf\n", s1->lowerLeft->x, s1->lowerLeft->y, s1->upperRight->x, s1->upperRight->y );
  printf( "wanted: %lf %lf %lf %lf\n", s2->lowerLeft->x, s2->lowerLeft->y, s2->upperRight->x, s2->upperRight->y );

  EXPECT_TRUE( n->sons[SW]->goNorth(0.4375) ==
               n->sons[NW]->sons[SE]->sons[SE]);
  
  EXPECT_TRUE( n->sons[SW]->goNorth(0.4375)->goNorth(0.4375) ==
               n->sons[NW]->sons[SE]->sons[NE]);

  EXPECT_TRUE( n->sons[SW]->goNorth(0.4375)->goNorth(0.4375)->goNorth(0.4375) ==
               n->sons[NW]->sons[NE]);

  EXPECT_TRUE( n->sons[SW]->goNorth(0.4375)->goNorth(0.4375)->goNorth(0.4375)->goNorth(0.4375) == NULL);
}

list<Node *> *getLeafNodes(Node *root){
    list<Node *> *results = new list<Node *>;

    if(root->type != GRAY){
      results->push_back(root);
      return results;
    }

    for( Quadrant q : quadrantList ){
      list<Node *> *aux = getLeafNodes(root->sons[q]);
      results->insert( results->end(), aux->begin(), aux->end() );
      delete aux;
    }

    return results;
}

TEST_F(PMQuadTreeTest, searchInterior) {
  GIMSPoint *lowerLeft = new GIMSPoint(0,0),
            *upperRight = new GIMSPoint(1,1);
  GIMSBoundingBox *box = new GIMSBoundingBox(lowerLeft, upperRight);

  PMQuadTree *tree = new PMQuadTree(box);
  Node *n = tree->root;
  n->split();
  n->sons[NW]->split();
  n->sons[NW]->sons[SE]->split();
  n->sons[NW]->sons[SW]->split();
  n->sons[NW]->sons[NW]->split();
  n->sons[NW]->sons[SW]->sons[NW]->split();
  n->sons[NW]->sons[NE]->split();

  GIMSPoint *points[6] = {new GIMSPoint(0.0625, 0.5625),
                          new GIMSPoint(0.0625, 0.9375),
                          new GIMSPoint(0.3175, 0.9375),
                          new GIMSPoint(0.3175, 0.8020),
                          new GIMSPoint(0.4525, 0.9375),
                          new GIMSPoint(0.4525, 0.5675)};

  GIMSGeometryList *extRing = new GIMSGeometryList();
  for(int i=0; i<6; i++){
    if( i < 5 )
      extRing->list->push_back( new GIMSEdge(points[i], points[i+1]) );
    else
      extRing->list->push_back( new GIMSEdge(points[i], points[0]) );
  }
  GIMSPolygon *p = new GIMSPolygon(extRing, NULL);
  GIMSGeometryList *newList = new GIMSGeometryList();
  newList->list->push_back(p);

  tree->insert(newList);
  list<Node *> *results = (list<Node *> *)(tree->root->searchInterior(p));

  //if everything is ok, every node in the north west quadrant of the root
  //is reported in the results

  //lets then fetch all leaf nodes in the north west quadrant of the root node
  list<Node *> *nodes = getLeafNodes(tree->root->sons[NW]);

  bool allin = true;
  for( list<Node *>::iterator i = results->begin(); i != results->end(); i++ ){
    bool inside = false;
    for( list<Node *>::iterator j = nodes->begin(); j != nodes->end(); j++ ){
      if( (*j) == (*i) ){
        inside = true;
        break;
      }
    }
    if(!inside){
      allin = false;
      break;
    }
  }

  //check if all root->NW leaf nodes are returned
  EXPECT_TRUE(allin);

  //now lets confirm that root->NE,SE,SW are not in the query results
  bool allout = true;
  for( list<Node *>::iterator i = results->begin(); i != results->end(); i++ ){
    if( ((Node *)(*i)) == tree->root->sons[NW] || 
        ((Node *)(*i)) == tree->root->sons[SE] ||
        ((Node *)(*i)) == tree->root->sons[SW] )
      allout = false;
  }
  EXPECT_TRUE(allout);

  //The following code renders the pmqtree and highlights the reported nodes
  /*
  tree->query = newList;
  renderer = new DebRenderer();
  renderer->renderCallback = tree;
  char *argv[] = {"gims"};
  renderer->mainloop(1, argv);
  */
}






}  // namespace
