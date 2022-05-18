#pragma once

#define MAX_VARCHAR_SIZE (128 * 1024 * 1024)

typedef struct _varchar
{
    int length;
    char *data;
} varchar;

varchar *varchar_new(char *);

void varchar_del(varchar *);
