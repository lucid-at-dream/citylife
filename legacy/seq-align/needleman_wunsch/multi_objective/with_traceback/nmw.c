#include "nmw.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


int is_dominated(score_t *a, score_t *b)
{
    if (a->indel >= b->indel && a->match <= b->match)
    {
        return -1; // A is dominated by B or A and B are equal
    }

    if (b->indel >= a->indel && b->match <= a->match)
    {
        return 1; // B is dominated by A
    }

    return 0; // Neither solution dominates the other
}

// TODO: Can be improved from N^2 to NlogN
void filter_pareto_front(list *solutions)
{
    list_node *to_remove[solutions->size];
    int to_remove_count = 0;

    for (list_node *i = solutions->head; i != NULL; i = i->next)
    {
        score_t *i_v = (score_t *)(i->value);
        for (list_node *j = solutions->head; j != NULL; j = j->next)
        {
            if (i == j)
            {
                continue;
            }

            score_t *j_v = (score_t *)(j->value);

            int result = is_dominated(i_v, j_v);
            if (result < 0)
            {
                to_remove[to_remove_count++] = i;
                break;
            }
        }
    }

    for (int i = 0; i < to_remove_count; i++)
    {
        free(to_remove[i]->value);
        list_del_node(solutions, to_remove[i]);
    }
}

list *nmw(seq_align_problem *p)
{
    // Instantiate (i.e. allocate memory for) the matrix
    list ***matrix = (list ***)calloc(p->N + 1, sizeof(list *));
    for (int i = 0; i <= p->N; i++)
    {
        matrix[i] = (list **)calloc(p->M + 1, sizeof(list *));
        for (int j = 0; j <= p->M; j++)
        {
            matrix[i][j] = list_new();
        }
    }

    // Initialize first column and first row with values
    {
        score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
        zero_score->prev_i = zero_score->prev_j = 0;
        zero_score->prev_node = NULL;
        zero_score->indel = 0;
        zero_score->match = 0;
        list_append(matrix[0][0], zero_score);
    }

    for (int i = 1; i <= p->N; i++)
    {
        score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
        zero_score->prev_i = i - 1;
        zero_score->prev_j = 0;
        zero_score->prev_node = matrix[i-1][0]->head->value;
        zero_score->indel = i;
        zero_score->match = 0;
        list_append(matrix[i][0], zero_score);
    }

    for (int j = 1; j <= p->M; j++)
    {
        score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
        zero_score->prev_i = 0;
        zero_score->prev_j = j - 1;
        zero_score->prev_node = matrix[0][j-1]->head->value;
        zero_score->indel = j;
        zero_score->match = 0;
        list_append(matrix[0][j], zero_score);
    }

    // Build the full matrix
    for (int i = 1; i <= p->N; i++)
    {
        for (int j = 1; j <= p->M; j++)
        {
            for (list_node *k = matrix[i-1][j-1]->head; k != NULL; k = k->next)
            {
                score_t *diag = (score_t *)calloc(1, sizeof(score_t));
                diag->prev_i = i - 1;
                diag->prev_j = j - 1;
                diag->prev_node = k->value;
                diag->indel = ((score_t *)(k->value))->indel;
                if (p->A[i-1] == p->B[j-1])
                {
                    diag->match = ((score_t *)(k->value))->match + 1;
                }
                else
                {
                    diag->match = ((score_t *)(k->value))->match;
                }
                list_append(matrix[i][j], diag);
            }

            for (list_node *k = matrix[i-1][j]->head; k != NULL; k = k->next)
            {
                score_t *top = (score_t *)calloc(1, sizeof(score_t));
                top->prev_i = i - 1;
                top->prev_j = j;
                top->prev_node = k->value;
                top->indel = ((score_t *)(k->value))->indel + 1;
                top->match = ((score_t *)(k->value))->match;
                list_append(matrix[i][j], top);
            }

            for (list_node *k = matrix[i][j-1]->head; k != NULL; k = k->next)
            {
                score_t *left = (score_t *)calloc(1, sizeof(score_t));
                left->prev_i = i;
                left->prev_j = j - 1;
                left->prev_node = k->value;
                left->indel = ((score_t *)(k->value))->indel + 1;
                left->match = ((score_t *)(k->value))->match;
                list_append(matrix[i][j], left);
            }

            filter_pareto_front(matrix[i][j]);
        }
    }

    // Traceback each of the final undominated alignments
    list *pareto_front = list_new();
    for (list_node *s = matrix[p->N][p->M]->head; s != NULL; s = s->next)
    {
        int k = 0;
        int i = p->N;
        int j = p->M;

        char buffer_A[p->N + p->M + 1];
        memset(buffer_A, '\0', p->N + p->M + 1);

        char buffer_B[p->N + p->M + 1];
        memset(buffer_B, '\0', p->N + p->M + 1);

        score_t *partial_solution = (score_t *)(s->value);
        while (i > 0 || j > 0)
        {
            if (partial_solution->prev_i != i && partial_solution->prev_j != j)
            {
                // diagonal move
                buffer_A[k] = p->A[partial_solution->prev_i];
                buffer_B[k] = p->B[partial_solution->prev_j];
            }
            else if (partial_solution->prev_i != i)
            {
                buffer_A[k] = p->A[partial_solution->prev_i];
                buffer_B[k] = '-';
            }
            else
            {
                buffer_A[k] = '-';
                buffer_B[k] = p->B[partial_solution->prev_j];
            }

            i = partial_solution->prev_i;
            j = partial_solution->prev_j;
            k++;
            partial_solution = partial_solution->prev_node;
        }

        // Reverse
        for (int i = 0; k-1-i > i; i++)
        {
            char tmp = buffer_A[k-1-i];
            buffer_A[k-1-i] = buffer_A[i];
            buffer_A[i] = tmp;

            tmp = buffer_B[k-1-i];
            buffer_B[k-1-i] = buffer_B[i];
            buffer_B[i] = tmp;
        }

        bi_seq_align_solution *solution = (bi_seq_align_solution *)calloc(1, sizeof(bi_seq_align_solution));
        
        solution->score = (score_t *)calloc(1, sizeof(score_t));
        solution->score->indel = ((score_t *)(s->value))->indel;
        solution->score->match = ((score_t *)(s->value))->match;

        solution->len = k;
        
        solution->traceback_A = (char *)calloc(k + 1, sizeof(char));
        sprintf(solution->traceback_A, "%s", buffer_A);

        solution->traceback_B = (char *)calloc(k + 1, sizeof(char));
        sprintf(solution->traceback_B, "%s", buffer_B);

        list_append(pareto_front, solution);
    }

    // list *solution = nmw_traceback((list ***)matrix, p);

    for (int i = 0; i <= p->N; i++)
    {
        for(int j = 0; j <= p->M; j++)
        {
            for (list_node *n = matrix[i][j]->head; n != NULL; n = n->next)
            {
                free(n->value);
            }
            list_destroy(matrix[i][j]);
        }
        free(matrix[i]);
    }
    free(matrix);

    return pareto_front;
}

