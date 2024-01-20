#include <stdio.h>
#include <stdlib.h>

Node *createNode(double point[2], int axis)
{
    Node *newNode = (Node *)malloc(sizeof(Node));
    if (newNode != NULL)
    {
        newNode->point[0] = point[0];
        newNode->point[1] = point[1];
        newNode->axis = axis;
        newNode->left = NULL;
        newNode->right = NULL;
    }
    return newNode;
}

void insert(Node **node, double point[2], int axis)
{
    if (*node == NULL)
    {
        *node = createNode(point, axis);
    }
    else
    {
        if (point[axis] < (*node)->point[axis])
        {
            insert(&(*node)->left, point, (axis + 1) % 2);
        }
        else
        {
            insert(&(*node)->right, point, (axis + 1) % 2);
        }
    }
}

void insertKDTree(KDTree *tree, double point[2])
{
    insert(&(tree->root), point, 0);
}

// Example usage
int main()
{
    KDTree kdtree;
    kdtree.root = NULL;

    double points_to_insert[][2] = {{2, 3}, {5, 4}, {9, 6}, {4, 7}, {8, 1}, {7, 2}};
    int num_points = sizeof(points_to_insert) / sizeof(points_to_insert[0]);

    for (int i = 0; i < num_points; ++i)
    {
        insertKDTree(&kdtree, points_to_insert[i]);
    }

    // Your kd-tree is ready for use

    return 0;
}
