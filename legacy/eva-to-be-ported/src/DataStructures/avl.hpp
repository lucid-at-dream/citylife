#ifndef _AVL_HPP_
#define _AVL_HPP_

#include <cstdlib>
#include <iostream>
#include <list>
#include <cmath>
#include "Geometry.hpp"

#define KEYCMP(x, y) ((x) > (y) ? 1 : (x) < (y) ? -1 : 0)

using namespace GIMS_GEOMETRY;
using namespace std;

class AVLNode {
public:
    AVLNode *parent;
    AVLNode *left;
    AVLNode *right;

    int height;
    GIMS_Geometry *data;

    //constructor
    AVLNode(GIMS_Geometry *data);
    //destructor
    ~AVLNode();
    //tree like printing function
    void prettyPrint();
    //prints the elements in preorder
    void preOrder();
    //updates the node height
    void updateHeight();
    //returns the balance factor of the tree rooted at this node
    int getBalance();
    //retrieves the data corresponding to the given key
    GIMS_Geometry *find(long long key);
    //inserts a given item in the tree
    int insert(GIMS_Geometry *item);
    /*TODO. height update is buggy!!*/
    AVLNode *remove(long long item);
    void rebalanceAfterRemove();
    void rotateLeft();
    void rotateRight();
};

class AVLTree {
private:
    int nnodes;
    AVLNode *root;

public:
    /*container iterator*/
    class iterator {
    private:
        AVLNode *node;

    public:
        iterator(AVLNode *n) {
            this->node = n;
        }

        iterator operator++(int) {
            this->next();
            return *this;
        }

        bool operator==(const iterator &rhs) {
            return this->equals(rhs);
        }

        bool operator!=(const iterator &rhs) {
            return !(this->equals(rhs));
        }

        GIMS_Geometry *operator*() {
            return this->getData();
        }

        void next() {
            /*the next in order sucessor is the leftmost leaf of the right subtree.*/
            if (node->right != NULL) {
                this->node = this->node->right;
                while (this->node->left != NULL)
                    this->node = this->node->left;

                /*if there's no right subtree, then we need to go up.*/
            } else {
                if (this->node->parent != NULL) {
                    if (this->node == this->node->parent->left)
                        this->node = this->node->parent;
                    else {
                        while (this->node->parent != NULL && this->node == this->node->parent->right)
                            this->node = this->node->parent;
                        this->node = this->node->parent;
                    }
                } else {
                    this->node = NULL;
                }
            }
        }

        bool equals(iterator it) {
            if (this->node == it.node)
                return true;
            return false;
        }

        GIMS_Geometry *getData() {
            return node->data;
        }
    };

    //function definitions
    iterator begin();
    iterator end();
    void preOrder();
    void prettyPrint();
    GIMS_Geometry *top();
    AVLTree();
    ~AVLTree();
    int size();
    void merge(AVLTree *tree);
    GIMS_Geometry *find(long long key);
    void insert(GIMS_Geometry *item);
    AVLNode *remove(long long item);
};

#endif
