#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "test.h"
#include "assert.h"

void setup_env() {
}

char before_test() {
    return 0;
}

char after_test() {
    return 0;
}

void clean_env() {
}

typedef struct q_op {
    char type;
    char *data;
} q_op;

char test_queue_add_pop() {

    q_op ops[] = {
        {-1, NULL},
        {-1, NULL},
        {1, "a"},
        {1, "b"},
        {1, "c"},
        {-1, "a"},
        {-1, "b"},
        {-1, "c"},
        {-1, NULL},
        {1, "aa"},
        {1, "bb"},
        {1, "cc"},
        {-1, "aa"},
        {-1, "bb"},
        {1, "dd"},
        {-1, "cc"},
        {-1, "dd"},
        {1, "mem check"}
    };

    int length = sizeof(ops) / sizeof(q_op);

    queue *q = queue_new();

    for (int i = 0; i < length; i++) {
        
        if (ops[i].type == 1) {
            printf("Adding \"%s\" to queue\n", ops[i].data);
            queue_add(q, ops[i].data);
        
        } else {
            printf("Poping \"%s\" from queue\n", ops[i].data);
            char *data = (char *)queue_pop(q);

            int assertion_error = assert_str_equals("The item that popped from the queue should be the expected one.", data, ops[i].data);
            if (assertion_error) {
                return 1;
            }
        }
    }

    queue_del(q);
    return 0;
}

test test_suite[] = {
    {
        "Test adding and popping several items from a queue", test_queue_add_pop
    }
};

int main(int argc, char **argv) {
    suite_report report = run_test_suite(test_suite, sizeof(test_suite)/sizeof(test));

    if (report.failures > 0) {
        return -1;
    }

    return 0;
}
