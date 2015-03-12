#ifndef _AVL_HPP_
#define _AVL_HPP_

#include <cstdlib>
#include <iostream>
#include <list>
#include <cmath>

using namespace std;

template <class KEY, class DATATYPE>
class AVLNode{
    public:
        AVLNode<KEY, DATATYPE> *parent;
        AVLNode<KEY, DATATYPE> *left;
        AVLNode<KEY, DATATYPE> *right;

        int height;
        DATATYPE data;
        KEY (*getKey)(DATATYPE item);
        int (*cmp)(KEY a, KEY b);

        //constructor
        AVLNode(DATATYPE data, int (*cmp)(KEY a, KEY b), KEY (*getKey)(DATATYPE item) ){
            this->data = data;
            this->cmp = cmp;
            this->getKey = getKey;
            this->left = NULL;
            this->right = NULL;
            this->parent = NULL;
            this->height = 1;
        }

        //destructor
        ~AVLNode(){
            if(this->left != NULL)
                delete this->left;
            if(this->right != NULL)
                delete this->right;
        }

        //tree like printing function
        void prettyPrint(){
            list<pair<int, AVLNode *> > queue = list<pair<int, AVLNode *> >();
            queue.push_back(pair<int, AVLNode *>(0,this));

            int inc = pow( 2, log(this->height)/log(2.0) );
            int prevLevel = 0, currLevel = 0;

            while(queue.size() > 0){

                pair<int, AVLNode *> curr = queue.front();

                prevLevel = currLevel;
                currLevel = curr.first;
                if( currLevel != prevLevel ){
                    cout << endl;
                    inc = pow( 2, log(this->height - currLevel)/log(2.0) );
                }

                for(int i=0; i<inc; i++)
                    cout << " ";

                if( curr.second == NULL ){
                    queue.remove(curr);
                    continue;
                }

                cout << getKey(curr.second->data);
                queue.remove(curr);

                queue.push_back(pair<int, AVLNode *>( currLevel + 1, curr.second->left ) );
                queue.push_back(pair<int, AVLNode *>( currLevel + 1, curr.second->right) );
            }

        }

        //prints the elements in preorder
        void preOrder(){
            cout << this->data << "(" << height << "), ";
            if(this->left != NULL)
                this->left->preOrder();
            if(this->right != NULL)
                this->right->preOrder();
        }

        //updates the node height
        void updateHeight(){
            int hleft  = this->left  != NULL ? this->left->height : 0,
                hright = this->right != NULL ? this->right->height: 0;
            this->height = hleft > hright ? hleft + 1 : hright + 1;
        }

        //returns the balance factor of the tree rooted at this node
        int getBalance(){
            int hleft  = this->left  != NULL ? this->left->height : 0,
                hright = this->right != NULL ? this->right->height: 0;
            return hleft - hright;
        }

        //retrieves the data corresponding to the given key
        DATATYPE find(KEY key){
            AVLNode *aux = this;
            int compar;
            while(aux != NULL){
                compar = cmp(key, getKey(aux->data));
                if( compar > 0 ){
                    aux = aux->right;
                }else if( compar < 0 ){
                    aux = aux->left;
                }else{
                    return aux->data;
                }
            }
            return (DATATYPE)NULL;
        }

        //inserts a given item in the tree
        int insert(DATATYPE item){

            int compar = cmp(getKey(item), getKey(this->data));

            int added = 0;
            if(compar > 0){
                if(this->right == NULL){
                    this->right = new AVLNode<KEY, DATATYPE>(item, cmp, getKey);
                    this->right->parent = this;
                    added = 1;
                }else
                    added = this->right->insert(item); //check for nullness
            }else if(compar < 0){
                if(this->left == NULL){
                    this->left = new AVLNode<KEY, DATATYPE>(item, cmp, getKey);
                    this->left->parent = this;
                    added = 1;
                }else
                    added = this->left->insert(item); //check for nullness
            }

            //update node height
            int hleft  = this->left  != NULL ? this->left->height : 0,
                hright = this->right != NULL ? this->right->height: 0;
            this->height = hleft > hright ? hleft + 1 : hright + 1;

            //check if this subtree is unbalanced
            int balance = hleft - hright;

            //rebalance
            if( balance > 1 || balance < -1 ){
                // Left Left Case
                if (balance >  1 && cmp(getKey(item), getKey(this->left->data)) < 0 )
                    this->rotateRight();

                // Left Right Case
                else if (balance >  1 && cmp(getKey(item), getKey(this->left->data)) > 0 ) {
                    this->left->rotateLeft();
                    this->rotateRight();
                }

                // Right Right Case
                else if (balance < -1 && cmp(getKey(item), getKey(this->right->data)) > 0 )
                    this->rotateLeft();

                // Right Left Case
                else if (balance < -1 && cmp(getKey(item), getKey(this->right->data)) < 0 ) {
                    this->right->rotateRight();
                    this->rotateLeft();
                }
            }

            return added;
        }

        /*TODO. height update is buggy!!*/
        AVLNode *remove(KEY item){

            AVLNode *removedNode = NULL;

            int compar = cmp(item, getKey(this->data));

            if(compar > 0){
                if(this->right != NULL)
                    removedNode = this->right->remove(item); //check for nullness
                return removedNode;
            }else if(compar < 0){
                if(this->left != NULL)
                    removedNode = this->left->remove(item); //check for nullness
                return removedNode;

            }else{ /*this is the node to remove*/
                removedNode = this;

                /*if there is no offspring, we can delete right away*/
                if( this->left == NULL && this->right == NULL ){
                    if(this->parent != NULL){
                        if( this == this->parent->left )
                            this->parent->left = NULL;
                        else
                            this->parent->right = NULL;
                    }
                /*if there's only one child, we can just replace this node for it*/
                }else if( this->left != NULL && this->right == NULL ){
                    if( this->parent != NULL ){
                        if( this->parent->right == this )
                            this->parent->right = this->left;
                        else
                            this->parent->left = this->left;
                    }
                    this->left->parent = this->parent;

                }else if( this->left == NULL && this->right != NULL ){
                    if( this->parent != NULL ){
                        if( this->parent->right == this )
                            this->parent->right = this->right;
                        else
                            this->parent->left = this->right;
                    }
                    this->right->parent = this->parent;

                }else{
                    /*if we have two children, we need to fetch the leftmost node of the 
                      right subtree and replace this node for it.*/
                    
                    /*find it*/
                    AVLNode *leftmost = this->right;
                    while(leftmost->left != NULL)
                        leftmost = leftmost->left;

                    /*remove it*/
                    if( leftmost->parent->right == leftmost ){
                        leftmost->parent->right = NULL;
                        if( leftmost->right != NULL ){
                            leftmost->parent->right = leftmost->right;
                            leftmost->right->parent = leftmost->parent;
                        }
                    }else{
                        leftmost->parent->left = NULL;
                        if( leftmost->right != NULL ){
                            leftmost->parent->left = leftmost->right;
                            leftmost->right->parent = leftmost->parent;
                        }
                    }

                    /*replace ourselves*/
                    if( this->parent != NULL ){
                        if( this->parent->right == this )
                            this->parent->right = leftmost;
                        else
                            this->parent->left = leftmost;
                    }

                    leftmost->parent = this->parent;
                    
                    leftmost->right = this->right;
                    if(leftmost->right != NULL)
                        leftmost->right->parent = leftmost;
                    
                    leftmost->left = this->left;
                    if(leftmost->left != NULL)
                        leftmost->left->parent = leftmost;
                }
            }

            AVLNode *startLeaf = this->left ? this->left : this->right ? this->right : this->parent;
            AVLNode *aux = startLeaf;

            if(aux != NULL){
                while(aux->parent != NULL)
                    aux = aux->parent;
                
                int compar = 0;
                while( aux != NULL ){
                    startLeaf = aux;
                    compar = cmp(item, getKey(aux->data));
                    if(compar > 0)
                        aux = aux->right;
                    else if(compar < 0)
                        aux = aux->left;
                }

                while(startLeaf->parent != NULL){
                    startLeaf->rebalanceAfterRemove();
                    startLeaf = startLeaf->parent;
                }
            }

            return removedNode;
        }

        void rebalanceAfterRemove(){

            //update node height
            int hleft  = this->left  != NULL ? this->left->height : 0,
                hright = this->right != NULL ? this->right->height: 0;
            this->height = hleft > hright ? hleft + 1 : hright + 1;

            //check if this subtree is unbalanced
            int balance = hleft -hright;
 
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

            if(this->parent != NULL)
                this->parent->rebalanceAfterRemove();
        }

        void rotateLeft(){
            // Perform rotation
            AVLNode *aux = this->right->left;
            this->right->left = this;
            this->right->parent = this->parent;
            if(this->parent != NULL){
                if(this->parent->left == this)
                    this->parent->left = this->right;
                else
                    this->parent->right = this->right;
            }
            this->parent = this->right;
            this->right = aux;
            if(aux != NULL)
                aux->parent = this;
         
            //  Update heights
            this->updateHeight();
            this->parent->updateHeight();
        }

        void rotateRight(){
            // Perform rotation
            AVLNode *aux = this->left->right;
            this->left->right = this;
            this->left->parent = this->parent;
            if(this->parent != NULL){
                if(this->parent->left == this)
                    this->parent->left = this->left;
                else
                    this->parent->right = this->left;
            }
            this->parent = this->left;
            this->left = aux;
            if(aux != NULL)
                aux->parent = this;

            // Update heights
            this->updateHeight();
            this->parent->updateHeight();
        }

};


template <class KEY, class DATATYPE>
class AVLTree{
    private:
        int nnodes;
        AVLNode<KEY, DATATYPE> *root;
        KEY (*getKey)(DATATYPE item);
        int (*cmp)(KEY a, KEY b);

    public:

        class iterator{
            private:
                AVLNode<KEY, DATATYPE> *node;

            public:
                iterator(AVLNode<KEY, DATATYPE> *n){
                    this->node = n;
                }

                iterator operator++(int) {
                    this->next();
                    return *this;
                }

                bool operator==(const iterator& rhs){
                    return this->equals(rhs);
                }

                bool operator!=(const iterator& rhs){
                    return !(this->equals(rhs));
                }

                DATATYPE operator* (){
                    return this->getData();
                }


                void next(){
                    /*the next in order sucessor is the leftmost leaf of the right subtree.*/
                    if(node->right != NULL){
                        this->node = this->node->right;
                        while(this->node->left != NULL)
                            this->node = this->node->left;
                    
                    /*if there's no right subtree, then we need to go up.*/
                    }else{
                        if( this->node->parent != NULL ){
                            if(this->node == this->node->parent->left)
                                this->node = this->node->parent;
                            else{
                                while(this->node->parent != NULL && this->node == this->node->parent->right)
                                    this->node = this->node->parent;
                                this->node = this->node->parent;
                            }
                        }else{
                            this->node = NULL;
                        }
                    }
                }

                bool equals(iterator it){
                    if(this->node == it.node)
                        return true;
                    return false;
                }

                DATATYPE getData(){
                    return node->data;
                }
        };

        iterator begin(){
            if(this->root == NULL)
                return this->end();
            AVLNode<KEY, DATATYPE> *aux = this->root;
            while(aux->left != NULL){
                aux = aux->left;
            }
            return iterator(aux);
        }

        iterator end(){
            return iterator(NULL);
        }

        void preOrder(){
            this->root->preOrder();
        }

        void prettyPrint(){
            this->root->prettyPrint();
        }

        DATATYPE top(){
            if(this->root == NULL)
                return (DATATYPE)NULL;
            return this->root->data;
        }

        //constructor
        AVLTree(int (*cmp)(KEY a, KEY b), KEY (*getKey)(DATATYPE item)){
            this->root = NULL;
            this->cmp = cmp;
            this->getKey = getKey;
            this->nnodes = 0;
        }

        //destructor
        ~AVLTree(){
            if(this->root != NULL)
                delete this->root;
        }

        //function definitions
        int size(){
            return this->nnodes;
        }

        /*TODO: !Optimize! this can be done in linear time with in order traversals*/
        //merges the given tree into this tree.
        void merge(AVLTree<KEY, DATATYPE> *tree) {
            for(AVLTree<KEY, DATATYPE>::iterator it = tree->begin(); it != tree->end(); it++)
                this->insert(*it);    
        }

        DATATYPE find(KEY key){
            if(this->root == NULL)
                return (DATATYPE)NULL;
            return this->root->find(key);
        }

        void insert(DATATYPE item){
            //insert
            if( this->root == NULL ){
                this->root = new AVLNode<KEY, DATATYPE>(item, this->cmp, this->getKey);
                this->nnodes = 1;
            }else
                this->nnodes += this->root->insert(item);
            //update root
            while(this->root->parent != NULL)
                this->root = this->root->parent;
        }

        AVLNode<KEY, DATATYPE> *remove(KEY item){
            if(this->root == NULL)
                return NULL;

            AVLNode<KEY, DATATYPE> *rm = this->root->remove(item);
            if( rm != NULL ){
                this->nnodes--;

                if( this->root == rm )
                    this->root = rm->right != NULL ? rm->right : rm->left;

                if(this->root != NULL)
                    while(this->root->parent != NULL)
                        this->root = this->root->parent;

            }
            return rm;
        }
};


#endif
