#include "seq_align.h"
#include "list.h"

struct _score_t;

typedef struct _score_t {
    struct _score_t *prev_node;
    int prev_i, prev_j;
    int indel;
    int match;
} score_t;

typedef struct
{
    score_t *score;
    unsigned len;
    char *traceback_A;
    char *traceback_B;
} bi_seq_align_solution;

list *hirschberg_mo(seq_align_problem *p);

