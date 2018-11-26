#include "assert.h"

#include <stdio.h>
#include <string.h>

char assert_str_equals(char *assertion, char *str1, char *str2) {
  int cmp = strcmp(str1, str2);
  
  if (cmp != 0) {
    printf("%s > got [%s], expected [%s]\n", assertion, str1, str2);
    return 1;
  }
  return 0;
}

