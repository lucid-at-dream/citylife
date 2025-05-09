#include "seq_align.h"
#include "nmw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    seq_align_problem problem = read_input(argv[1]);

    seq_align_solution solution = nmw(&problem);

    printf("%ld\n", (long)(solution.score));
    printf("%s\n", solution.traceback_A);
    printf("%s\n", solution.traceback_B);

    free_problem(&problem);
    free_solution(&solution);

    return 0;
}

