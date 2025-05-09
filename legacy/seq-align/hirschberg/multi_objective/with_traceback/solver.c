#include "seq_align.h"
#include "hirsch.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void report_solution(void **item)
{
    bi_seq_align_solution *solution = (bi_seq_align_solution *)(*item);
    printf("indel: %d, match: %d\n", solution->score->indel, solution->score->match);
    printf("%s\n", solution->traceback_A);
    printf("%s\n", solution->traceback_B);
    printf("\n");
}

int main(int argc, char **argv)
{
    seq_align_problem problem = read_input(argv[1]);

    list *pareto_front = hirschberg_mo(&problem);

    list_foreach(pareto_front, report_solution);

    free_problem(&problem);
    for(list_node *n = pareto_front->head; n != NULL; n = n->next)
    {
        bi_seq_align_solution *solution = n->value;
        free(solution->traceback_A);
        free(solution->traceback_B);
        free(solution->score);
        free(solution);
    }
    list_destroy(pareto_front);

    return 0;
}

