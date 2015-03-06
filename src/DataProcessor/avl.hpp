#ifndef _AVL_HPP_
#define _AVL_HPP_

#include <cstdlib>
#include <iostream>

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

        void preOrder(){
            cout << this->data << "(" << height << "), ";
            if(this->left != NULL)
                this->left->preOrder();
            if(this->right != NULL)
                this->right->preOrder();
        }

        void updateHeight(){
            int hleft  = this->left  != NULL ? this->left->height : 0,
                hright = this->right != NULL ? this->right->height: 0;
            this->height = hleft > hright ? hleft + 1 : hright + 1;
        }

        //function definitions
        int insert(DATATYPE item){

            int compar = cmp(getKey(item), getKey(this->data));

            int added = 0;
            if(compar > 0){
                if(this->right == NULL){
                    cout << "adding " << item << " to the right" << endl;
                    this->right = new AVLNode<KEY, DATATYPE>(item, cmp, getKey);
                    this->right->parent = this;
                    added = 1;
                }else
                    added = this->right->insert(item); //check for nullness
            }else if(compar < 0){
                if(this->left == NULL){
                    cout << "adding " << item << " to the left" << endl;
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
            int balance = hleft -hright;

            //rebalance
            if( balance > 1 || balance < -1 ){
                cout << "rebalancing" << endl;

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

        /*TODO. Rebalancing*/
        int remove(KEY item){

            int compar = cmp(item, getKey(this->data));

            if(compar > 0){
                if(this->right == NULL)
                    return 0;
                else
                    return this->right->remove(item); //check for nullness
            }else if(compar < 0){
                if(this->left == NULL)
                    return 0;
                else
                    return this->left->remove(item); //check for nullness
            }

            /*if there are is offspring, we can delete right away*/

/*TODO: watch out for parent nullness when node is the root*/

            /*if there's only one child, we can just replace this node for it*/
            if( this->left != NULL && this->right == NULL ){
                if( this->parent->right == this )
                    this->parent->right = this->left;
                else
                    this->parent->left = this->left;
                this->left->parent = this->parent;

            }else if( this->left == NULL && this->right != NULL ){
                if( this->parent->right == this )
                    this->parent->right = this->right;
                else
                    this->parent->left = this->right;
                this->right->parent = this->parent;

            }else{
                /*if we have two children, we need to fetch the leftmost node of the 
                  right subtree and replace this node for it.*/
                
                /*find it*/
                AVLNode *leftmost = this->right;
                while(leftmost->left != NULL)
                    leftmost = leftmost->left;

                /*remove it*/
                if( leftmost->right != NULL ){
                    leftmost->parent->left = leftmost->right;
                    leftmost->right->parent = leftmost->parent;
                }

                /*replace ourselves*/
                if( this->parent->right == this )
                    this->parent->right = leftmost;
                else
                    this->parent->left = leftmost;
                leftmost->parent = this->parent;
            }

            delete this;
            return -1;
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
                    this->parent->left = this->right;
                else
                    this->parent->right = this->right;
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

        void preOrder(){
            this->root->preOrder();
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
        void size(){
            return this->nnodes;
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

        void remove(KEY item){
            //remove
            this->nnodes += this->root->remove(item);
            //update root
            while(this->root->parent != NULL)
                this->root = this->root->parent;
        }
};


#endif
