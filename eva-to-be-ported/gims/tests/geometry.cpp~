#include "GIMSGeometry.hpp"
#include "PMQuadTree.hpp"
#include "gtest/gtest.h"
#include <cmath>

using namespace GIMSGEOMETRY;

namespace {

// The fixture for testing class Foo.
class GIMSGeometryTest : public ::testing::Test {
 protected:
  // You can remove any or all of the following functions if its body
  // is empty.

  GIMSGeometryTest() {
    // You can do set-up work for each test here.
  }

  virtual ~GIMSGeometryTest() {
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

// Tests particular cases of input to function distanceSquared2p
TEST_F(GIMSGeometryTest, distanceSquared2p) {
  GIMSPoint *p1 = new GIMSPoint(1,0),
            *p2 = new GIMSPoint(1,1),
            *p3 = new GIMSPoint(2,2);

  EXPECT_EQ(1, distanceSquared2p(p1,p2));
  EXPECT_EQ(2, distanceSquared2p(p2,p3));
  EXPECT_EQ(5, distanceSquared2p(p1,p3));
}

// Tests particular cases of input to function angle3p
TEST_F(GIMSGeometryTest, angle3p) {
  GIMSPoint *p1 = new GIMSPoint(1,0),
            *p2 = new GIMSPoint(1,1),
            *p3 = new GIMSPoint(2,2);

  EXPECT_EQ(M_PI/4.0 * 3.0, angle3p(p1,p2,p3));
}

// Tests particular cases of input to function cosine3p
TEST_F(GIMSGeometryTest, cosine3p) {
  GIMSPoint *p1 = new GIMSPoint(1,0),
            *p2 = new GIMSPoint(1,1),
            *p3 = new GIMSPoint(2,2);

  EXPECT_EQ(-1.0/sqrt(2), cosine3p(p1,p2,p3));
}

// Tests particular cases of input to function sideOf
TEST_F(GIMSGeometryTest, sideOf) {
  GIMSPoint *p1 = new GIMSPoint(1,0),
            *p2 = new GIMSPoint(1,1),
            *p3 = new GIMSPoint(2,2),
            *p4 = new GIMSPoint(3,3);

  GIMSEdge *e12 = new GIMSEdge(p1, p2),
           *e13 = new GIMSEdge(p1, p3),
           *e23 = new GIMSEdge(p2, p3),
           *e21 = new GIMSEdge(p2, p1),
           *e31 = new GIMSEdge(p3, p1),
           *e32 = new GIMSEdge(p3, p2);

  EXPECT_EQ(RIGHT, p3->sideOf(e12));
  EXPECT_EQ(LEFT, p3->sideOf(e21));

  EXPECT_EQ(LEFT, p2->sideOf(e13));
  EXPECT_EQ(RIGHT, p2->sideOf(e31));

  EXPECT_EQ(RIGHT, p1->sideOf(e23));
  EXPECT_EQ(LEFT, p1->sideOf(e32));

  EXPECT_EQ(ALIGNED, p4->sideOf(e23));
  EXPECT_EQ(ALIGNED, p4->sideOf(e32));
}

// Tests particular cases of input to function insideBox
TEST_F(GIMSGeometryTest, isInsideBox) {
  GIMSPoint *p1 = new GIMSPoint(1.5,0),   //outside
            *p2 = new GIMSPoint(0,1.5),   //outside
            *p3 = new GIMSPoint(1.5,1.5), //inside (middle)
            *p4 = new GIMSPoint(2,1.5),   //inside (border)
            *p5 = new GIMSPoint(1,1.5),   //inside (border)
            *p6 = new GIMSPoint(1.5,2),   //inside (border)
            *p7 = new GIMSPoint(1.5,1);   //inside (border)

  GIMSPoint *lowerLeft = new GIMSPoint(1,1),
            *upperRight = new GIMSPoint(2,2);
  GIMSBoundingBox *box = new GIMSBoundingBox(lowerLeft, upperRight);

  EXPECT_FALSE(p1->isInsideBox(box));
  EXPECT_FALSE(p2->isInsideBox(box));
  EXPECT_TRUE (p3->isInsideBox(box));
  EXPECT_TRUE (p4->isInsideBox(box));
  EXPECT_TRUE (p5->isInsideBox(box));
  EXPECT_TRUE (p6->isInsideBox(box));
  EXPECT_TRUE (p7->isInsideBox(box));
}

TEST_F(GIMSGeometryTest, distToSegmentSquared){
  GIMSPoint *p1 = new GIMSPoint(1,0), //dist to p3-p4 = sqrt(2)
            *p2 = new GIMSPoint(1,1), //dist to p3-p4 = 1
            *p3 = new GIMSPoint(2,2), //dist to p1-p2 = sqrt(2)
            *p4 = new GIMSPoint(2,1); //dist to p2-p3 = sqrt(2)/2
  //to note that this distances have to be squared!

  GIMSEdge *e12 = new GIMSEdge(p1, p2),
           *e13 = new GIMSEdge(p1, p3),
           *e23 = new GIMSEdge(p2, p3),
           *e21 = new GIMSEdge(p2, p1),
           *e34 = new GIMSEdge(p3, p4);

  EXPECT_EQ(2, distToSegmentSquared(p1,e34));
  EXPECT_EQ(1, distToSegmentSquared(p2,e34));
  EXPECT_EQ(2, distToSegmentSquared(p3,e12));
  EXPECT_EQ(2, distToSegmentSquared(p3,e21));
  EXPECT_EQ(1/2.0, distToSegmentSquared(p4,e23));

  EXPECT_EQ(distToSegmentSquared(p2,e13), distToSegmentSquared(p4,e13));

}


}  // namespace
