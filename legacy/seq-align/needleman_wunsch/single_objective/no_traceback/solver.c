#include "seq_align.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int nmw(seq_align_problem *p);

int main(int argc, char **argv)
{
    seq_align_problem problem = read_input(argv[1]);

    int score = nmw(&problem);

    printf("%d\n", score);

    free_problem(&problem);

    return 0;
}

int nmw(seq_align_problem *p)
{
    int *prev = (int *)calloc(p->M + 1, sizeof(int));
    int *curr = (int *)calloc(p->M + 1, sizeof(int));

    for (int j = 0; j <= p->M; j++)
    {
        prev[j] = -j;
    }
    curr[0] = -1;

    for (int i = 1; i <= p->N; i++)
    {
        for (int j = 1; j <= p->M; j++)
        {
            int diag, left, top;

            if (p->A[i-1] == p->B[j-1])
                diag = prev[j-1] + 1;
            else
                diag = prev[j-1] - 1;

            top = prev[j] - 1;
            left = curr[j-1] - 1;

            if (diag >= left && diag >= top)
                curr[j] = diag;
            else if (left >= diag && left >= top)
                curr[j] = left;
            else if (top >= diag && top >= left)
                curr[j] = top;
        }

        int *tmp = prev;
        prev = curr;
        curr = tmp;
        curr[0] = -i - 1;
    }

    int score = prev[p->M];
    free(curr);
    free(prev);

    return score;
}

