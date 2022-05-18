#pragma once

typedef struct _bigint
{
    long unsigned int *words;
    unsigned int nwords;
} bigint;

bigint bigint_new(long initial_value);
void bigint_del(bigint);

void bigint_multiply(bigint, bigint);
void bigint_divide(bigint, bigint);
void bigint_add(bigint, bigint);
void bigint_subtract(bigint, bigint);
void bigint_increment(bigint);
void bigint_decrement(bigint);

bigint bigint_clone(bigint);

char *bigint_tostring(bigint);