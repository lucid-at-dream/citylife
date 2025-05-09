#include "seq_align.h"

#include <stdio.h>
#include <stdlib.h>

void free_problem(seq_align_problem *problem)
{
    free(problem->A);
    free(problem->B);
    problem->N = problem->M = 0;
    problem->A = problem->B = NULL;
}

void free_solution(seq_align_solution *solution)
{
    free(solution->traceback_A);
    free(solution->traceback_B);
}

seq_align_problem read_input(char *input_file)
{
    int N = 0, M = 0;
    char *A = NULL, *B = NULL;

    FILE *f = fopen(input_file, "r");
    char in;
    char buffer[1024 * 1024];
    int i = 0;
    size_t read_bytes;
    while ((read_bytes = fread(&in, sizeof(char), 1, f)) > 0)
    {
        if (in != '\n')
        {
            buffer[i] = in;
            buffer[i + 1] = '\0';
            i += 1;
        }

        if (in == '\n')
        {
            if (A == NULL && B == NULL)
            {
                sscanf(buffer, "%d %d", &N, &M);
                A = calloc(N + 1, sizeof(char));
            }
            else if (B != NULL)
            {
                sprintf(B, "%s", buffer);
            }
            else if (A != NULL)
            {
                sprintf(A, "%s", buffer);
                B = calloc(M + 1, sizeof(char));
            }
            i = 0;
        }
    }

    fclose(f);

    return (seq_align_problem){N,M,A,B};
}

