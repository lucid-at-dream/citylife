#include "avl.h"

#include <stdlib.h>

avl_node *avl_node_new(void *data);
void avl_node_destroy(avl_node *node);

void tree_node_destroy_recurse(avl_node *node);
void tree_node_insert_recurse(avl_node *node, avl_node *new_node, int (*compare)(const void *, const void *));
avl_node *tree_node_find(avl_node *this, void *data, int (*compare)(const void *, const void *));
void rotateLeft(avl_node *this);
void rotateRight(avl_node *this);

avl_tree *tree_new(int (*compare)(const void *, const void *))
{
    avl_tree *new = (avl_tree *)calloc(1, sizeof(avl_tree));
    new->compare = compare;
    return new;
}

void tree_destroy(avl_tree *tree)
{
    tree_node_destroy_recurse(tree->root);
    free(tree);
}

void tree_insert(avl_tree *tree, void *data)
{
    avl_node *new_node = avl_node_new(data);

    if (tree->root == NULL)
    {
        tree->root = new_node;
    }
    else
    {
        tree_node_insert_recurse(tree->root, new_node, tree->compare);
    }
}

avl_node *tree_find(avl_tree *this, void *data, int (*compare)(const void *given_data, const void *node_data))
{
    avl_node *node = this->root;

    while (node != NULL)
    {
        int relation = compare(data, node->data);
        if (relation > 0)
        {
            node = node->right;
        }
        else if (relation < 0)
        {
            node = node->left;
        }
        else
        {
            return node;
        }
    }
    return NULL;
}

void tree_remove(avl_tree *tree, avl_node *node)
{
    // TODO
}

avl_tree *tree_merge(avl_tree *a_tree, avl_tree *another_tree, char overwrite, void *(*merge)(const void *a_tree_data, const void *another_tree_data),
                     int (*compare)(const void *a_tree_data, const void *another_tree_data))
{
    // TODO
}

// Private functions

avl_node *avl_node_new(void *data)
{
    avl_node *new = (avl_node *)calloc(1, sizeof(avl_node));
    new->data = data;
    return new;
}

void avl_node_destroy(avl_node *node)
{
    free(node);
}

void tree_node_destroy_recurse(avl_node *node)
{
    if (node == NULL)
    {
        return;
    }

    tree_node_destroy_recurse(node->left);
    tree_node_destroy_recurse(node->right);

    avl_node_destroy(node);
}

void tree_node_insert_recurse(avl_node *this, avl_node *new_node, int (*compare)(const void *, const void *))
{
    int compar = compare(new_node->data, this->data);

    // Insert the node
    if (compar > 0)
    {
        if (this->right == NULL)
        {
            this->right = new_node;
            new_node->parent = this;
        }
        else
        {
            tree_node_insert_recurse(this->right, new_node, compare);
        }
    }
    else if (compar < 0)
    {
        if (this->left == NULL)
        {
            this->left = new_node;
            new_node->parent = this;
        }
        else
        {
            tree_node_insert_recurse(this->left, new_node, compare);
        }
    }

    // Update node height
    int hleft = this->left != NULL ? this->left->height : 0, hright = this->right != NULL ? this->right->height : 0;
    this->height = hleft > hright ? hleft + 1 : hright + 1;

    // Check if this subtree is unbalanced
    int balance = hleft - hright;

    // rebalance
    if (abs(balance) > 1)
    {
        // Left branch is too long
        if (balance > 1)
        {
            int relation = compare(new_node->data, this->left->data);
            if (relation > 0) // Node was inserted in the right subtree of the left node. Rotate subtree left.
            {
                rotateLeft(this->left);
            }
            rotateRight(this);
        }
        else // balance < 1: Right branch is too long
        {
            int relation = compare(new_node->data, this->right->data);
            if (relation < 0)
            {
                rotateRight(this->right); // Node was inserted in the left subtree of the right node. Rotate it right.
            }
            rotateLeft(this);
        }
    }
}

void rotateLeft(avl_node *this)
{
    /*
    // Perform rotation
    AVLNode *aux = this->right->left;
    this->right->left = this;
    this->right->parent = this->parent;
    if (this->parent != NULL)
    {
        if (this->parent->left == this)
            this->parent->left = this->right;
        else
            this->parent->right = this->right;
    }
    this->parent = this->right;
    this->right = aux;
    if (aux != NULL)
        aux->parent = this;

    //  Update heights
    this->updateHeight();
    this->parent->updateHeight();
    */
}

void rotateRight(avl_node *this)
{
    /*
    // Perform rotation
    AVLNode *aux = this->left->right;
    this->left->right = this;
    this->left->parent = this->parent;
    if (this->parent != NULL)
    {
        if (this->parent->left == this)
            this->parent->left = this->left;
        else
            this->parent->right = this->left;
    }
    this->parent = this->left;
    this->left = aux;
    if (aux != NULL)
        aux->parent = this;

    // Update heights
    this->updateHeight();
    this->parent->updateHeight();
    */
}

/*

//tree like printing function
//void prettyPrint();
//prints the elements in preorder
//void preOrder();
//updates the node height
//void updateHeight();
//returns the balance factor of the tree rooted at this node
//int getBalance();


//retrieves the data corresponding to the given key
void *avl_find(avl *tree, void *data);

//inserts a given item in the tree
int insert(GIMS_Geometry *item);
AVLNode *remove(long long item);
void rebalanceAfterRemove();
void rotateLeft();
void rotateRight();


//constructor
AVLNode::AVLNode(GIMS_Geometry *data)
{
    this->data = data;
    this->left = NULL;
    this->right = NULL;
    this->parent = NULL;
    this->height = 1;
}

//destructor
AVLNode::~AVLNode()
{
    if (this->left != NULL)
        delete this->left;
    if (this->right != NULL)
        delete this->right;
}

//tree like printing function
void AVLNode::prettyPrint()
{
    list<pair<int, AVLNode *> > queue = list<pair<int, AVLNode *> >();
    queue.push_back(pair<int, AVLNode *>(0, this));

    int inc = pow(2, log(this->height) / log(2.0));
    int prevLevel = 0, currLevel = 0;

    while (queue.size() > 0)
    {
        pair<int, AVLNode *> curr = queue.front();

        prevLevel = currLevel;
        currLevel = curr.first;
        if (currLevel != prevLevel)
        {
            cout << endl;
            inc = pow(2, log(this->height - currLevel) / log(2.0));
        }

        for (int i = 0; i < inc; i++)
            cout << " ";

        if (curr.second == NULL)
        {
            queue.remove(curr);
            continue;
        }

        queue.remove(curr);

        queue.push_back(pair<int, AVLNode *>(currLevel + 1, curr.second->left));
        queue.push_back(pair<int, AVLNode *>(currLevel + 1, curr.second->right));
    }
}

//prints the elements in preorder
void AVLNode::preOrder()
{
    cout << this->data << "(" << height << "), ";
    if (this->left != NULL)
        this->left->preOrder();
    if (this->right != NULL)
        this->right->preOrder();
}

//updates the node height
void AVLNode::updateHeight()
{
    int hleft = this->left != NULL ? this->left->height : 0, hright = this->right != NULL ? this->right->height : 0;
    this->height = hleft > hright ? hleft + 1 : hright + 1;
}

//returns the balance factor of the tree rooted at this node
int AVLNode::getBalance()
{
    int hleft = this->left != NULL ? this->left->height : 0, hright = this->right != NULL ? this->right->height : 0;
    return hleft - hright;
}

//retrieves the data corresponding to the given key
GIMS_Geometry *AVLNode::find(long long key)
{
}


AVLNode *AVLNode::remove(long long item)
{
    AVLNode *removedNode = NULL;

    int compar = KEYCMP(item, this->data->id);

    if (compar > 0)
    {
        if (this->right != NULL)
            removedNode = this->right->remove(item); //check for nullness
        return removedNode;
    }
    else if (compar < 0)
    {
        if (this->left != NULL)
            removedNode = this->left->remove(item); //check for nullness
        return removedNode;
    }
    else
    {
        removedNode = this;

        //if there is no offspring, we can delete right away
        if (this->left == NULL && this->right == NULL)
        {
            if (this->parent != NULL)
            {
                if (this == this->parent->left)
                    this->parent->left = NULL;
                else
                    this->parent->right = NULL;
            }
            //if there's only one child, we can just replace this node for it
        }
        else if (this->left != NULL && this->right == NULL)
        {
            if (this->parent != NULL)
            {
                if (this->parent->right == this)
                    this->parent->right = this->left;
                else
                    this->parent->left = this->left;
            }
            this->left->parent = this->parent;
        }
        else if (this->left == NULL && this->right != NULL)
        {
            if (this->parent != NULL)
            {
                if (this->parent->right == this)
                    this->parent->right = this->right;
                else
                    this->parent->left = this->right;
            }
            this->right->parent = this->parent;
        }
        else
        {
            //if we have two children, we need to fetch the leftmost node of the 
            //right subtree and replace this node for it.

            // find it
            AVLNode *leftmost = this->right;
            while (leftmost->left != NULL)
                leftmost = leftmost->left;

            // remove it
            if (leftmost->parent->right == leftmost)
            {
                leftmost->parent->right = NULL;
                if (leftmost->right != NULL)
                {
                    leftmost->parent->right = leftmost->right;
                    leftmost->right->parent = leftmost->parent;
                }
            }
            else
            {
                leftmost->parent->left = NULL;
                if (leftmost->right != NULL)
                {
                    leftmost->parent->left = leftmost->right;
                    leftmost->right->parent = leftmost->parent;
                }
            }

            // replace ourselves
            if (this->parent != NULL)
            {
                if (this->parent->right == this)
                    this->parent->right = leftmost;
                else
                    this->parent->left = leftmost;
            }

            leftmost->parent = this->parent;

            leftmost->right = this->right;
            if (leftmost->right != NULL)
                leftmost->right->parent = leftmost;

            leftmost->left = this->left;
            if (leftmost->left != NULL)
                leftmost->left->parent = leftmost;
        }
    }

    AVLNode *startLeaf = this->left ? this->left : this->right ? this->right : this->parent;
    AVLNode *aux = startLeaf;

    if (aux != NULL)
    {
        while (aux->parent != NULL)
            aux = aux->parent;

        int compar = 0;
        while (aux != NULL)
        {
            startLeaf = aux;
            compar = KEYCMP(item, aux->data->id);
            if (compar > 0)
                aux = aux->right;
            else if (compar < 0)
                aux = aux->left;
        }

        while (startLeaf->parent != NULL)
        {
            startLeaf->rebalanceAfterRemove();
            startLeaf = startLeaf->parent;
        }
    }

    return removedNode;
}

void AVLNode::rebalanceAfterRemove()
{
    //update node height
    int hleft = this->left != NULL ? this->left->height : 0, hright = this->right != NULL ? this->right->height : 0;
    this->height = hleft > hright ? hleft + 1 : hright + 1;

    //check if this subtree is unbalanced
    int balance = hleft - hright;

    // If this node becomes unbalanced, then there are 4 cases

    // Left Left Case
    if (balance > 1 && this->left->getBalance() >= 0)
        this->rotateRight();

    // Left Right Case
    if (balance > 1 && this->left->getBalance() < 0)
    {
        this->left->rotateLeft();
        this->rotateRight();
    }

    // Right Right Case
    if (balance < -1 && this->right->getBalance() <= 0)
        this->rotateLeft();

    // Right Left Case
    if (balance < -1 && this->right->getBalance() > 0)
    {
        this->right->rotateRight();
        this->rotateLeft();
    }

    if (this->parent != NULL)
        this->parent->rebalanceAfterRemove();
}

AVLTree::iterator AVLTree::begin()
{
    if (this->root == NULL)
        return this->end();
    AVLNode *aux = this->root;
    while (aux->left != NULL)
    {
        aux = aux->left;
    }
    return iterator(aux);
}

AVLTree::iterator AVLTree::end()
{
    return iterator(NULL);
}

void AVLTree::preOrder()
{
    this->root->preOrder();
}

void AVLTree::prettyPrint()
{
    this->root->prettyPrint();
}

GIMS_Geometry *AVLTree::top()
{
    if (this->root == NULL)
        return NULL;
    return this->root->data;
}

//constructor
AVLTree::AVLTree()
{
    this->root = NULL;
    this->nnodes = 0;
}

//destructor
AVLTree::~AVLTree()
{
    if (this->root != NULL)
        delete this->root;
}

//function definitions
int AVLTree::size()
{
    return this->nnodes;
}

//merges the given tree into this tree.
void AVLTree::merge(AVLTree *tree)
{
    for (AVLTree::iterator it = tree->begin(); it != tree->end(); it++)
        this->insert(*it);
}

GIMS_Geometry *AVLTree::find(long long key)
{
    if (this->root == NULL)
        return NULL;
    return this->root->find(key);
}

void AVLTree::insert(GIMS_Geometry *item)
{
    //insert
    if (this->root == NULL)
    {
        this->root = new AVLNode(item);
        this->nnodes = 1;
    }
    else
        this->nnodes += this->root->insert(item);
    //update root
    while (this->root->parent != NULL)
        this->root = this->root->parent;
}

AVLNode *AVLTree::remove(long long item)
{
    if (this->root == NULL)
        return NULL;

    AVLNode *rm = this->root->remove(item);
    if (rm != NULL)
    {
        this->nnodes--;

        if (this->root == rm)
            this->root = rm->right != NULL ? rm->right : rm->left;

        if (this->root != NULL)
            while (this->root->parent != NULL)
                this->root = this->root->parent;
    }
    return rm;
}
*/