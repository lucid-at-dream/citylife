#pragma once

typedef struct Node
{
    double point[2];
    int axis;
    struct Node *left;
    struct Node *right;
} Node;

typedef struct KDTree
{
    Node *root;
} KDTree;