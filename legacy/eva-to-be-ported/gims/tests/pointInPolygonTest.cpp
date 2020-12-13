#include "Geometry.hpp"
#include "PMQuadTree.hpp"
#include "gtest/gtest.h"
#include <cstdio>
#include <cstring>
#include <iostream>

#define RENDERTEST 111

#define INPUTS_DIR "tests/cases/ptInsidePol/inputs/"
#define OUTPUTS_DIR "tests/cases/ptInsidePol/outputs/"

using namespace GIMS_GEOMETRY;
using namespace PMQUADTREE;
using namespace std;

namespace
{
// The fixture for testing class Foo.
class PtInPolTest : public ::testing::TestWithParam<int> {
    public:
        char cases_input[30][100] = {
                INPUTS_DIR "1_0",    INPUTS_DIR "1_1",    INPUTS_DIR "1_2",    INPUTS_DIR "1_3",    INPUTS_DIR "1_4",
                INPUTS_DIR "1_5",    INPUTS_DIR "1_6",    INPUTS_DIR "1_7",    INPUTS_DIR "1_8",    INPUTS_DIR "1_9",
                INPUTS_DIR "100_0",  INPUTS_DIR "100_1",  INPUTS_DIR "100_2",  INPUTS_DIR "100_3",  INPUTS_DIR "100_4",
                INPUTS_DIR "100_5",  INPUTS_DIR "100_6",  INPUTS_DIR "100_7",  INPUTS_DIR "100_8",  INPUTS_DIR "100_9",
                INPUTS_DIR "5000_0", INPUTS_DIR "5000_1", INPUTS_DIR "5000_2", INPUTS_DIR "5000_3", INPUTS_DIR "5000_4",
                INPUTS_DIR "5000_5", INPUTS_DIR "5000_6", INPUTS_DIR "5000_7", INPUTS_DIR "5000_8", INPUTS_DIR "5000_9"
        };
        char cases_output[30][100] = { OUTPUTS_DIR "1_0",    OUTPUTS_DIR "1_1",    OUTPUTS_DIR "1_2",
                                       OUTPUTS_DIR "1_3",    OUTPUTS_DIR "1_4",    OUTPUTS_DIR "1_5",
                                       OUTPUTS_DIR "1_6",    OUTPUTS_DIR "1_7",    OUTPUTS_DIR "1_8",
                                       OUTPUTS_DIR "1_9",    OUTPUTS_DIR "100_0",  OUTPUTS_DIR "100_1",
                                       OUTPUTS_DIR "100_2",  OUTPUTS_DIR "100_3",  OUTPUTS_DIR "100_4",
                                       OUTPUTS_DIR "100_5",  OUTPUTS_DIR "100_6",  OUTPUTS_DIR "100_7",
                                       OUTPUTS_DIR "100_8",  OUTPUTS_DIR "100_9",  OUTPUTS_DIR "5000_0",
                                       OUTPUTS_DIR "5000_1", OUTPUTS_DIR "5000_2", OUTPUTS_DIR "5000_3",
                                       OUTPUTS_DIR "5000_4", OUTPUTS_DIR "5000_5", OUTPUTS_DIR "5000_6",
                                       OUTPUTS_DIR "5000_7", OUTPUTS_DIR "5000_8", OUTPUTS_DIR "5000_9" };
};

// Tests particular cases of input to function goNorth
TEST_P(PtInPolTest, polygonContainsPoint)
{
        int curr_test = GetParam();

        char *inputText = (char *)malloc(sizeof(char) * 1000000);
        FILE *in = fopen(cases_input[curr_test], "r");
        fread(inputText, sizeof(char), 5000000, in);

        /*read number of points*/
        int N = atoi(strtok(inputText, "\n"));
        GIMS_Point **points = (GIMS_Point **)malloc(sizeof(GIMS_Point *) * N);

        /*read polygon as well known text (wkt)*/
        char *wkt = strtok(NULL, "\n");
        GIMS_Polygon *p = (GIMS_Polygon *)fromWkt(wkt);
        p->id = 1;
        GIMS_BoundingBox *box = p->getExtent();

        /*read the points*/
        for (int i = 0; i < N; i++) {
                wkt = strtok(NULL, "\n");
                points[i] = (GIMS_Point *)fromWkt(wkt);
        }

        /*run the test*/
        PMQuadTree *tree = new PMQuadTree(box);
        tree->insert(p);
        char *results = (char *)malloc(N * sizeof(char));
        for (int i = 0; i < N; i++) {
                results[i] = tree->contains(p, points[i]) ? '1' : '0';
        }

        if (curr_test == RENDERTEST) {
                for (int i = 0; i < N; i++) {
                        if (results[i] == '0') {
                                tree->renderRed(points[i]);
                        } else {
                                tree->renderGreen(points[i]);
                        }
                }

                tree->query = p;
                renderer = new DebRenderer();
                renderer->setScale(400.0 / box->xlength(), -400.0 / box->ylength());
                renderer->setTranslation(-box->minx(), -box->maxy());
                renderer->renderCallback = tree;
                int argc = 1;
                char *argv[] = { "gims", "test" };
                renderer->mainloop(argc, argv);
        }

        /*read expected output file*/
        FILE *out = fopen(cases_output[curr_test], "r");
        int size = fread(inputText, sizeof(char), 5000000, out);
        inputText[size] = '\0';

        /*compare the results*/
        wkt = strtok(inputText, "\n");
        EXPECT_EQ(wkt[0], results[0]);
        for (int i = 1; i < N; i++) {
                wkt = strtok(NULL, "\n");
                EXPECT_EQ(wkt[0], results[i]);
                delete points[i];
        }

        free(inputText);
        free(points);
        free(results);
}
} // namespace

INSTANTIATE_TEST_CASE_P(DefaultTestInstantiation, PtInPolTest, ::testing::Range(0, 30));
