#include "bigint.h"

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

bigint bigint_new(long initial_value)
{
    bigint number;
    number.words = (long unsigned int *)malloc(sizeof(long unsigned int));
    number.words[0] = initial_value;
    number.nwords = 1;
    return number;
}

void bigint_del(bigint number)
{
    free(number.words);
}

void bigint_add(bigint a, bigint b)
{
    a.words[0] += b.words[0];
}

void bigint_increment(bigint number)
{
    number.words[0]++;
}

char *bigint_tostring(bigint number)
{
    int ndigits = (int)ceil((number.nwords * 64) / log2(10));
    char *buffer = (char *)calloc(ndigits + 1, sizeof(char));
    snprintf(buffer, ndigits + 1, "%lu", number.words[0]);
    return buffer;
}
