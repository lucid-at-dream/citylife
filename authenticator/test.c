#include <stdio.h>
#include <stdlib.h>

#include "authenticator.h"

typedef struct _test {
  char *description;
  int (*test_impl) (void);
} test;

test tests[] = {
};

int main(int argc, char** argv) {

  int count = 0;
  for (; count < sizeof(tests) / sizeof(test); count++) {
    test *t = tests + count;
    printf("Runing test %d: '%s'\n", count + 1, t->description);
    t->test_impl();
  }

  printf("finished executing %d tests.\n", count);

  return 0;
}

