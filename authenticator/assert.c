#include "assert.h"

#include <stdio.h>
#include <string.h>

char assert_int_equals(char *assertion, int num1, int num2) {
  if (num1 != num2) {
    printf("%s > got [%d], expected [%d]\n", assertion, num1, num2);
    return 1;
  }
  return 0;
}

char assert_str_equals(char *assertion, char *str1, char *str2) {
  if (str1 == str2) {
    return 0;
  }
  
  int cmp = strcmp(str1, str2);
  
  if (cmp != 0) {
    printf("%s > got [%s], expected [%s]\n", assertion, str1, str2);
    return 1;
  }
  return 0;
}

char assert_float_less_than(char *assertion, float n1, float n2) {
  if (n1 < n2) {
    return 0;
  } else {
    printf("%s > expected [%f] to be less than [%f]\n", assertion, n1, n2);
    return 1;
  }
}

