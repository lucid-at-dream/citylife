#include "nmw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

seq_align_solution nmw(seq_align_problem *p)
{
    int **matrix = (int **)calloc(p->N + 1, sizeof(int**));
    for (int i = 0; i <= p->N; i++)
    {
        matrix[i] = (int *)calloc(p->M + 1, sizeof(int*));
    }

    matrix[0][0] = 0;

    for (int i = 1; i <= p->N; i++)
    {
        matrix[i][0] = -i;
    }

    for (int j = 1; j <= p->M; j++)
    {
        matrix[0][j] = -j;
    }

    for (int i = 1; i <= p->N; i++)
    {
        for (int j = 1; j <= p->M; j++)
        {
            int diag, left, top;

            if (p->A[i-1] == p->B[j-1])
                diag = matrix[i-1][j-1] + 1;
            else
                diag = matrix[i-1][j-1] - 1;

            left = matrix[i-1][j] - 1;
            top = matrix[i][j-1] - 1;

            if (diag >= left && diag >= top)
                matrix[i][j] = diag;
            else if (left >= diag && left >= top)
                matrix[i][j] = left;
            else if (top >= diag && top >= left)
                matrix[i][j] = top;
        }
    }

    seq_align_solution solution = nmw_traceback((int **)matrix, p);

    for (int i = 0; i <= p->N; i++)
    {
        free(matrix[i]);
    }
    free(matrix);

    return solution;
}

seq_align_solution nmw_traceback(int **matrix, seq_align_problem *p)
{
    char buffer_A[p->N + p->M + 1];
    char buffer_B[p->N + p->M + 1];
    buffer_A[p->N + p->M] = '\0';
    buffer_B[p->N + p->M] = '\0';

    int i = p->N, j = p->M;

    int idx = p->N + p->M - 1;

    while (i > 0 || j > 0)
    {
        if (i-1 >= 0 && matrix[i][j] == matrix[i-1][j] - 1)
        {
            buffer_A[idx] = p->A[i-1];
            buffer_B[idx] = '-';
            i--;
        }
        else if (j-1 >= 0 && matrix[i][j] == matrix[i][j-1] - 1)
        {
            buffer_A[idx] = '-';
            buffer_B[idx] = p->B[j-1];
            j--;
        }
        else
        {
            buffer_A[idx] = p->A[i-1];
            buffer_B[idx] = p->B[j-1];
            i--;
            j--;
        }
        idx--;
    }

    unsigned len = p->N + p->M - idx;

    char *traceback_A = (char *)calloc(p->N + p->M + 1, sizeof(char));
    sprintf(traceback_A, "%s", &(buffer_A[idx + 1]));

    char *traceback_B = (char *)calloc(p->N + p->M + 1, sizeof(char));
    sprintf(traceback_B, "%s", &(buffer_B[idx + 1]));

    seq_align_solution solution;
    solution.score = matrix[p->N][p->M];
    solution.len = len;
    solution.traceback_A = traceback_A;
    solution.traceback_B = traceback_B;

    return solution;
}
