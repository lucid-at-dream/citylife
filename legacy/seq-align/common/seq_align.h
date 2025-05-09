#pragma once

#define MAX(a,b) ((a) > (b) ? (a) : (b))

typedef struct
{
    int N, M;
    char *A, *B;
} seq_align_problem;

typedef struct
{
    int score;
    unsigned len;
    char *traceback_A;
    char *traceback_B;
} seq_align_solution;

seq_align_problem read_input(char *input_file);
void free_problem(seq_align_problem *problem);
void free_solution(seq_align_solution *solution);

