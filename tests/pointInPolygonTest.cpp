#include "Geometry.hpp"
#include "PMQuadTree.hpp"
#include "gtest/gtest.h"
#include <cstdio>
#include <cstring>

#define INPUTS_DIR "cases/ptInsidePol/inputs/"
#define OUTPUTS_DIR "cases/ptInsidePol/inputs/"

using namespace GIMSGEOMETRY;
using namespace PMQUADTREE;
using namespace std;

namespace {

    // The fixture for testing class Foo.
    class PtInPolTest : public ::testing::TestWithParam<int> {
        char[30][] cases_input = {
             INPUTS_DIR "1_0",
             INPUTS_DIR "1_1",
             INPUTS_DIR "1_2",
             INPUTS_DIR "1_3",
             INPUTS_DIR "1_4",
             INPUTS_DIR "1_5",
             INPUTS_DIR "1_6",
             INPUTS_DIR "1_7",
             INPUTS_DIR "1_8",
             INPUTS_DIR "1_9",
             INPUTS_DIR "100_0",
             INPUTS_DIR "100_1",
             INPUTS_DIR "100_2",
             INPUTS_DIR "100_3",
             INPUTS_DIR "100_4",
             INPUTS_DIR "100_5",
             INPUTS_DIR "100_6",
             INPUTS_DIR "100_7",
             INPUTS_DIR "100_8",
             INPUTS_DIR "100_9",
             INPUTS_DIR "5000_0",
             INPUTS_DIR "5000_1",
             INPUTS_DIR "5000_2",
             INPUTS_DIR "5000_3",
             INPUTS_DIR "5000_4",
             INPUTS_DIR "5000_5",
             INPUTS_DIR "5000_6",
             INPUTS_DIR "5000_7",
             INPUTS_DIR "5000_8",
             INPUTS_DIR "5000_9"
        }
        char[30][] cases_output = {
             OUTPUTS_DIR "1_0",
             OUTPUTS_DIR "1_1",
             OUTPUTS_DIR "1_2",
             OUTPUTS_DIR "1_3",
             OUTPUTS_DIR "1_4",
             OUTPUTS_DIR "1_5",
             OUTPUTS_DIR "1_6",
             OUTPUTS_DIR "1_7",
             OUTPUTS_DIR "1_8",
             OUTPUTS_DIR "1_9",
             OUTPUTS_DIR "100_0",
             OUTPUTS_DIR "100_1",
             OUTPUTS_DIR "100_2",
             OUTPUTS_DIR "100_3",
             OUTPUTS_DIR "100_4",
             OUTPUTS_DIR "100_5",
             OUTPUTS_DIR "100_6",
             OUTPUTS_DIR "100_7",
             OUTPUTS_DIR "100_8",
             OUTPUTS_DIR "100_9",
             OUTPUTS_DIR "5000_0",
             OUTPUTS_DIR "5000_1",
             OUTPUTS_DIR "5000_2",
             OUTPUTS_DIR "5000_3",
             OUTPUTS_DIR "5000_4",
             OUTPUTS_DIR "5000_5",
             OUTPUTS_DIR "5000_6",
             OUTPUTS_DIR "5000_7",
             OUTPUTS_DIR "5000_8",
             OUTPUTS_DIR "5000_9"
        }

        protected:
            PtInPolTest() {
            }
            virtual ~PMQuadTreeTest() {
            }
            virtual void SetUp() {
            }
            virtual void TearDown() {
            }
    };

    // Tests particular cases of input to function goNorth
    TEST_F(PMQuadTreeTest, goNorth) {

        char inputText[5000000];
        FILE *in = fopen(cases_input[curr_test], 'r');
        fread(inputText, sizeof(char), 5000000, in);

        /*read number of points*/
        int N = atoi(strtok(inputText, '\n'));

        /*read polygon as well known text (wkt)*/
        char *wkt = strtok(NULL, '\n');
        GIMS_Polygon *p = (GIMS_Polygon *)fromWkt(wkt); //TODO implement fromWkt method
        GIMS_BoundingBox *box = p->getExtent(); //TODO implement getExtent method

        /*read the points*/
        for(int i=0; i<N; i++){
            wkt = strtok(NULL, '\n');
            points[i] = (GIMS_Point *)fromWkt(wkt); //TODO implement fromWkt method
        }

        /*run the test*/
        PMQuadTree *tree = new PMQuadTree(box);
        tree->insert(p);
        char[N] results;
        for(int i=0; i<N; i++){
            results = tree->polygonContainsPoint(p, points[i]) ? '1' : '0';
        }

        /*read expected output file*/
        FILE *out = fopen(cases_output[curr_test], 'r');
        fread(inputText, sizeof(char), 5000000, out);

        /*compare the results*/
        char *wkt = strtok(inputText, '\n');
        EXPECT_EQ(wkt[0], resuts[0]);
        for(int i=0; i<N; i++){
            wkt = strtok(NULL, '\n');
            EXPECT_EQ(wkt[0], results[i]);
        }

        /*increment test file count*/
        curr_test++;
    }
}  // namespace

INSTANTIATE_TEST_CASE_P(DefaultTestInstantiation, PtInPolTest, ::testing::RANGE(0,30));
