#include "hirsch.h"
#include "list.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NMW_THRESHOLD 2

#define REVERSE 1
#define DONT_REVERSE 0

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
void filter_pareto_front_bi_solution(list *solutions)
{
    list_node *to_remove[solutions->size];
    int to_remove_count = 0;

    for (list_node *i = solutions->head; i != NULL; i = i->next)
    {
        score_t *i_v = ((bi_seq_align_solution *)(i->value))->score;
        for (list_node *j = solutions->head; j != NULL; j = j->next)
        {
            if (i == j)
            {
                continue;
            }

            score_t *j_v = ((bi_seq_align_solution *)(j->value))->score;

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


// TODO: This should be in a file of its own. Also, we need reverse ability here.
list **nmw_scorer(seq_align_problem *p, char reverse)
{
    list **prev = (list **)calloc(p->N + 1, sizeof(list *));
    list **current = (list **)calloc(p->N + 1, sizeof(list *));

    // Initialize first column and first row with values
    {
        score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
        zero_score->indel = 0;
        zero_score->match = 0;
        list_append(current[0], zero_score);
    }

    for (int i = 1; i <= p->N; i++)
    {
        score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
        zero_score->indel = i;
        zero_score->match = 0;
        list_append(current[i], zero_score);
    }

    // Build the full matrix
    for (int j = 1; j <= p->M; j++)
    {
        prev = current;
        current = prev;

        { // Initialize first column.
            score_t *zero_score = (score_t *)calloc(1, sizeof(score_t));
            zero_score->indel = j;
            zero_score->match = 0;

            current[0] = list_new();
            list_append(current[0], zero_score);
        }

        for (int i = 1; i <= p->N; i++)
        {
            current[i] = list_new();

            for (list_node *k = prev[i-1]->head; k != NULL; k = k->next)
            {
                score_t *diag = (score_t *)calloc(1, sizeof(score_t));
                diag->indel = ((score_t *)(k->value))->indel;

                char match;
                if (reverse == REVERSE)
                {
                    match = (p->A[p->N - i] == p->B[p->M - j]);
                }
                else
                {
                    match = (p->A[i-1] == p->B[j-1]);
                }

                if (match)
                {
                    diag->match = ((score_t *)(k->value))->match + 1;
                }
                else
                {
                    diag->match = ((score_t *)(k->value))->match;
                }
                list_append(current[i], diag);
            }

            for (list_node *k = prev[i]->head; k != NULL; k = k->next)
            {
                score_t *top = (score_t *)calloc(1, sizeof(score_t));
                top->indel = ((score_t *)(k->value))->indel + 1;
                top->match = ((score_t *)(k->value))->match;
                list_append(current[i], top);
            }

            for (list_node *k = current[i-1]->head; k != NULL; k = k->next)
            {
                score_t *left = (score_t *)calloc(1, sizeof(score_t));
                left->indel = ((score_t *)(k->value))->indel + 1;
                left->match = ((score_t *)(k->value))->match;
                list_append(current[i], left);
            }

            filter_pareto_front(current[i]);
        }

        // Clean up the "previous" line.
        for (int i = 0; i <= p->N; i++)
        {
            for (list_node *n = prev[i]->head; n != NULL; n = n->next)
            {
                free(n->value);
            }
            list_destroy(prev[i]);
        }
    }

    return current;
}

void list_merge(list *l, list *o)
{
    l->size += o->size;
    l->tail->next = o->head;
    l->tail = o->tail;
}

list *hirschberg_mo(seq_align_problem *p)
{
    list *solutions = NULL;

    if (p->N == 0)
    {
        solutions = list_new();
        bi_seq_align_solution *solution = (bi_seq_align_solution *)calloc(p->M + 1, sizeof(bi_seq_align_solution));

        solution->traceback_A = (char *)calloc(p->M + 1, sizeof(char));
        memset(solution->traceback_A, '-', p->M);

        solution->traceback_B = (char *)calloc(p->M + 1, sizeof(char));
        strncpy(solution->traceback_B, p->B, p->M);

        solution->score = (score_t *)calloc(1, sizeof(score_t));
        solution->score->indel = p->M;
        solution->score->match = 0;

        solution->len = p->M;
    }

    else if (p->M == 0)
    {
        solutions = list_new();
        bi_seq_align_solution *solution = (bi_seq_align_solution *)calloc(p->M + 1, sizeof(bi_seq_align_solution));

        solution->traceback_A = (char *)calloc(p->N + 1, sizeof(char));
        strncpy(solution->traceback_A, p->A, p->N);

        solution->traceback_B = (char *)calloc(p->N + 1, sizeof(char));
        memset(solution->traceback_B, '-', p->N);

        solution->score = (score_t *)calloc(1, sizeof(score_t));
        solution->score->indel = p->N;
        solution->score->match = 0;

        solution->len = p->N;
    }

    else if (p->N <= NMW_THRESHOLD || p->M <= NMW_THRESHOLD)
    {
        solutions = nmw(p);
    }

    else
    {

        // 1. Calc indices with non-dominated solutions.
        // 1.1) Scores left and right
        unsigned x_len = p->N,
            x_mid = x_len / 2;

        seq_align_problem subproblem_left = (seq_align_problem){
            x_mid,
            p->M,
            p->A,
            p->B
        };
        list **scores_left = nmw_scorer(&subproblem_left, DONT_REVERSE);

        seq_align_problem subproblem_right = (seq_align_problem){
            x_len - x_mid,
            p->M,
            p->A + x_mid,
            p->B
        };
        list **scores_right = nmw_scorer(&subproblem_right, REVERSE);

        // 1.2) Non empty set indices of non dominated sets of the cell-wise cartesian product of scores left and right
        list *cell_wise_product[p->M];

        // 1.2.1) cell-wise product
        for (int i = 0; i < p->M; i++)
        {
            list *product = list_new();
            for (list_node *n = scores_left[i]->head; n != NULL; n = n->next)
            {
                for (list_node *m = scores_right[p->M - i]->head; m != NULL; m = m->next)
                {
                    score_t *new_score = (score_t *)calloc(1, sizeof(score_t));
                    new_score->indel = ((score_t *)(n->value))->indel + ((score_t *)(m->value))->indel;
                    new_score->match = ((score_t *)(n->value))->match + ((score_t *)(m->value))->match;
                    list_append(product, new_score);
                }
            }
            filter_pareto_front(product);

            cell_wise_product[i] = product;
        }

        // 1.2.2) find indices with non dominated solutions
        int split_indices[p->M];
        int split_indices_count = 0;

        for (int i = 0; i < p->M; i++)
        {
            for (list_node *n = cell_wise_product[i]->head; n != NULL; n = n->next)
            {
                char n_is_non_dominated = 1;

                for (int j = 0; j < p->M && n_is_non_dominated; j++)
                {
                    if (i == j)
                    {
                        continue;
                    }

                    for (list_node *m = cell_wise_product[j]->head; m != NULL; m = m->next)
                    {
                        if (is_dominated(n->value, m->value) < 0)
                        {
                            n_is_non_dominated = 0;
                            break;
                        }
                    }
                }

                if (n_is_non_dominated)
                {
                    // Then i is a split index, no need to validate i any further, next i
                    split_indices[split_indices_count++] = i;
                    break;
                }
            }
        }

        list *solutions = list_new();
        for (int split_index_idx = 0; split_index_idx < split_indices_count; split_index_idx++)
        {
            int y_mid = split_indices[split_index_idx];

            subproblem_left = (seq_align_problem){
                x_mid,
                y_mid,
                p->A,
                p->B
            };
            list *partial_solution_left = hirschberg_mo(&subproblem_left);

            subproblem_right = (seq_align_problem){
                p->N - x_mid,
                p->M - y_mid,
                p->A + x_mid,
                p->B + y_mid
            };
            list *partial_solution_right = hirschberg_mo(&subproblem_right);

            // At this point we do the cartesian product of both sides and keep the undominated solutions.
            list *product = list_new();
            for (list_node *n = partial_solution_left->head; n != NULL; n = n->next)
            {
                for (list_node *m = partial_solution_right->head; m != NULL; m = m->next)
                {
                    bi_seq_align_solution *solution = (bi_seq_align_solution *)calloc(1, sizeof(bi_seq_align_solution));

                    score_t *new_score = (score_t *)calloc(1, sizeof(score_t));
                    new_score->indel = ((bi_seq_align_solution *)(n->value))->score->indel + ((bi_seq_align_solution *)(m->value))->score->indel;
                    new_score->match = ((bi_seq_align_solution *)(n->value))->score->match + ((bi_seq_align_solution *)(m->value))->score->match;
                    solution->score = new_score;

                    solution->len = ((bi_seq_align_solution *)(n->value))->len + ((bi_seq_align_solution *)(m->value))->len;
                    solution->traceback_A = (char *)calloc(solution->len + 1, sizeof(char));
                    solution->traceback_B = (char *)calloc(solution->len + 1, sizeof(char));
                    
                    sprintf(solution->traceback_A, "%s", ((bi_seq_align_solution *)(n->value))->traceback_A);
                    sprintf(solution->traceback_A + ((bi_seq_align_solution *)(n->value))->len, "%s", ((bi_seq_align_solution *)(m->value))->traceback_A);

                    sprintf(solution->traceback_B, "%s", ((bi_seq_align_solution *)(n->value))->traceback_B);
                    sprintf(solution->traceback_B + ((bi_seq_align_solution *)(n->value))->len, "%s", ((bi_seq_align_solution *)(m->value))->traceback_B);

                    list_append(product, solution);
                }
            }
            filter_pareto_front_bi_solution(product);

            list_merge(solutions, product);
        }

        filter_pareto_front_bi_solution(solutions);
    }

    return solutions;
}
