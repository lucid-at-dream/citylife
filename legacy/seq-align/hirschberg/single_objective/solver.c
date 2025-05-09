#include "seq_align.h"
#include "nmw.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMW_THRESHOLD 1000

#define REVERSE 1
#define DONT_REVERSE 0

seq_align_solution hirschberg(seq_align_problem *p);

int *nmw_score_prev = NULL;
int *nmw_score_curr = NULL;

int main(int argc, char **argv)
{
    seq_align_problem problem = read_input(argv[1]);

    nmw_score_prev = (int *)calloc(problem.N + problem.M + 1, sizeof(int));
    nmw_score_curr = (int *)calloc(problem.N + problem.M + 1, sizeof(int));

    seq_align_solution solution = hirschberg(&problem);

    printf("%d\n", solution.score);
    printf("%s\n", solution.traceback_A);
    printf("%s\n", solution.traceback_B);

    free_problem(&problem);
    free_solution(&solution);

    return 0;
}

int* nmw_score(seq_align_problem *p, char reverse)
{
    int *prev = nmw_score_prev;
    int *curr = nmw_score_curr;

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

            if (reverse == DONT_REVERSE)
            {
                if (p->A[i-1] == p->B[j-1])
                    diag = prev[j-1] + 1;
                else
                    diag = prev[j-1] - 1;
            }
            else
            {
                if (p->A[p->N - i] == p->B[p->M - j])
                    diag = prev[j-1] + 1;
                else
                    diag = prev[j-1] - 1;
            }

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

    return prev;
}

seq_align_solution hirschberg(seq_align_problem *p)
{
    seq_align_solution solution;

    if (p->N == 0)
    {
        solution.traceback_A = (char *)calloc(p->M + 1, sizeof(char));
        memset(solution.traceback_A, '-', p->M);

        solution.traceback_B = (char *)calloc(p->M + 1, sizeof(char));
        strncpy(solution.traceback_B, p->B, p->M);

        solution.score = -(p->M);
        solution.len = p->M;
    }

    else if (p->M == 0)
    {
        solution.traceback_A = (char *)calloc(p->N + 1, sizeof(char));
        strncpy(solution.traceback_A, p->A, p->N);

        solution.traceback_B = (char *)calloc(p->N + 1, sizeof(char));
        memset(solution.traceback_B, '-', p->N);

        solution.score = -(p->N);
        solution.len = p->N;
    }

    else if (p->N <= NMW_THRESHOLD || p->M <= NMW_THRESHOLD)
    {
        solution = nmw(p);
    }

    else
    {
        unsigned x_len = p->N,
            x_mid = x_len / 2;

        seq_align_problem subproblem_left = (seq_align_problem){
            x_mid,
            p->M,
            p->A,
            p->B
        };
        int *score_left = nmw_score(&subproblem_left, DONT_REVERSE);

        seq_align_problem subproblem_right = (seq_align_problem){
            x_len - x_mid,
            p->M,
            p->A + x_mid,
            p->B
        };
        int *score_right = nmw_score(&subproblem_right, REVERSE);

        unsigned y_mid = 0;
        int max = -999999;
        for (int i = 0; i <= p->M; i++)
        {
            int score = score_left[i] + score_right[p->M - i];
            if (score > max)
            {
                max = score;
                y_mid = i;
            }
        }

        subproblem_left = (seq_align_problem){
            x_mid,
            y_mid,
            p->A,
            p->B
        };
        seq_align_solution partial_solution_left = hirschberg(&subproblem_left);

        subproblem_right = (seq_align_problem){
            p->N - x_mid,
            p->M - y_mid,
            p->A + x_mid,
            p->B + y_mid
        };
        seq_align_solution partial_solution_right = hirschberg(&subproblem_right);

        seq_align_solution s = {
            .score = partial_solution_left.score + partial_solution_right.score,
            .len = partial_solution_left.len + partial_solution_right.len,
            .traceback_A = NULL,
            .traceback_B = NULL
        };

        s.traceback_A = (char *)calloc(s.len + 1, sizeof(char));
        s.traceback_B = (char *)calloc(s.len + 1, sizeof(char));

        int pA = sprintf(s.traceback_A, "%s", partial_solution_left.traceback_A);
        sprintf(s.traceback_A + pA, "%s", partial_solution_right.traceback_A);

        int pB = sprintf(s.traceback_B, "%s", partial_solution_left.traceback_B);
        sprintf(s.traceback_B + pB, "%s", partial_solution_right.traceback_B);

        free_solution(&partial_solution_left);
        free_solution(&partial_solution_right);

        solution = s;
    }

    return solution;
}

