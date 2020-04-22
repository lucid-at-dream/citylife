#include "bigint.h"

#include <limits.h>
#include <math.h>

bigint bigint_new(long initial_value) {
    bigint number;
    number.words = (long unsigned int *)malloc(sizeof(long unsigned int));
    number.words[0] = initial_value;
    number.nwords = 1;
    return number;
}

void bigint_del(bigint number) {
    free(number.words);
}

void bigint_increment(bigint number) {
    number.words[0]++;
}

char *bigint_tostring(bigint number) {
    int ndigits = ceil(log10(number.nwords * 64));
    char *buffer = (char *)calloc(ndigits, sizeof(char));
    sprintf(buffer, "%lu", number.words[0]);
    return buffer;
}
