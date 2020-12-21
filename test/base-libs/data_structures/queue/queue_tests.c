#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "test.h"
#include "assert.h"

typedef struct q_op {
    char type;
    char *data;
    int size;
} q_op;

char test_queue_add_pop()
{
    q_op ops[] = { { -1, NULL, 0 }, { -1, NULL, 0 }, { 1, "a", 1 },        { 1, "b", 2 },   { 1, "c", 3 },
                   { -1, "a", 2 },  { -1, "b", 1 },  { -1, "c", 0 },       { -1, NULL, 0 }, { 1, "aa", 1 },
                   { 1, "bb", 2 },  { 1, "cc", 3 },  { -1, "aa", 2 },      { -1, "bb", 1 }, { 1, "dd", 2 },
                   { -1, "cc", 1 }, { -1, "dd", 0 }, { 1, "mem check", 1 } };

    int length = sizeof(ops) / sizeof(q_op);

    queue *q = queue_new();

    for (int i = 0; i < length; i++) {
        if (ops[i].type == 1) {
            queue_add(q, ops[i].data);

        } else {
            char *data = (char *)queue_pop(q);

            int assertion_error = assert_str_equals("The item that popped from the queue should be the expected one.",
                                                    data, ops[i].data);
            if (assertion_error) {
                return 1;
            }
        }

        int assertion_error =
                assert_int_equals("The number of items in queue matches what is expected", q->size, ops[i].size);
        if (assertion_error) {
            return 1;
        }
    }

    queue_del(q);
    return 0;
}

test test_suite[] = { { "Test adding and popping several items from a queue", test_queue_add_pop } };

int main(int argc, char **argv)
{
    suite_report report = run_test_suite(test_suite, sizeof(test_suite) / sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
