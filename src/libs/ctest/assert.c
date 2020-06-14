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

char assert_null(char *assertion, void *ptr) {
  if (ptr != NULL) {
    printf("%s > got [%lu]", assertion, ptr);
    return 1;
  }
  return 0;
}

char assert_int_less_than(char *assertion, int num1, int num2) {
  if (num1 >= num2) {
    printf("%s > was expecting [%d] to be less than [%d]\n", assertion, num1, num2);
    return 1;
  }
  return 0;
}

char assert_str_equals(char *assertion, char *str1, char *str2) {
  if (str1 == str2) {
    return 0;
  }

  if (str1 == NULL || str2 == NULL) {
    printf("One of the strings is NULL.\n");
    return 1;
  }
  
  int cmp = strcmp(str1, str2);
  
  if (cmp != 0) {
    printf("%s > got [%s], expected [%s]\n", assertion, str1, str2);
    return 1;
  }
  return 0;
}

char assert_substr_in(char *assertion, char *str1, char *str2) {
  int size1 = strlen(str1), size2 = strlen(str2);
  
  if (size1 > size2) {
    printf("%s > [%s] not in [%s]\n", assertion, str1, str2);
    return 1;
  }

  int j;
  for (int i = 0; i + size1 < size2; i++) {
    
    j = 0; // Restart the number of matches counter

    if (str2[i] == str1[j]) { // First char matches

      for (j = 0; j < size1; j++) { // Iterate over substring
        if (str2[i+j] != str1[j]) {
          break; // No match
        }
      }

      if (j == size1) { // Went over the whole substring without a mismatch
        return 0;
      }
    }
  }

  printf("%s > [%s] not in [%s]\n", assertion, str1, str2);
  return 1;
}

char assert_float_less_than(char *assertion, float n1, float n2) {
  if (n1 < n2) {
    return 0;
  } else {
    printf("%s > expected [%f] to be less than [%f]\n", assertion, n1, n2);
    return 1;
  }
}

