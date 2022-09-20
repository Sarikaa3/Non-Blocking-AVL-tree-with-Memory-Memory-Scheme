#include<stdlib.h>
#include <vector>
#include <string>
#include <iostream>
#include <random>
#include <functional>
#include <thread>
#include <algorithm>
#include<bits/stdc++.h>
#include <mutex>
#include<pthread.h>
#include<atomic>

using namespace std; 
int c =0 ;
struct Node
{
	struct Node *parent;
	struct Node *left;
	struct Node *right;
	int key;
	bool is_leaf;
	bool is_root;
//	atomic<bool> flag;
	std::mutex lock;	
	int height;
	
};

int height(Node *N)
{
	        if (N == NULL)
			                return 0;
		        return N->height;
}
int getBalance(Node *N)
{
	if (N == NULL)
		return 0;
	return height(N->left) - height(N->right);
}
int max(int a, int b);

int max(int a, int b)
{
    return (a > b)? a : b;
}
 
Node* rootNode(int key)
{
    Node* node = new Node();
    node->key = key;
    node->left = NULL;
    node->right = NULL;
    node->height = 1; 
    node->is_leaf= false;
    node->is_root=true;
    node->parent=NULL;
                      
    return(node);
}
Node* newNode(int key, Node* parent)
{
	Node* node = new Node();
	node->key = key;
	node->left = NULL;
	node->right = NULL;
	node->height = 1;
	node->is_leaf= true;
	node->is_root=false;
	node->parent=parent;
        return(node);
}
Node *rightRotate(Node *y)
{
    Node *x = y->left;
    Node *T2 = x->right;
     x->right = y;
    y->left = T2;
 
    // Update heights
    y->height = max(height(y->left),
                    height(y->right)) + 1;
    x->height = max(height(x->left),
                    height(x->right)) + 1;
     return x;
}
 
Node *leftRotate(Node *x)
{
    Node *y = x->right;
    Node *T2 = y->left;
 
    y->left = x;
    x->right = T2;
 
       x->height = max(height(x->left),   
                    height(x->right)) + 1;
    y->height = max(height(y->left),
                    height(y->right)) + 1;
 
   
    return y;
}
void rightlock(Node* node){
	node->parent->lock.lock();
	node->lock.lock();
		        node->left->lock.lock();
			        node->left->right->lock.lock();
}
void rightunlock(Node* node){
	node->parent->lock.unlock();
	node->lock.unlock();
	        node->left->lock.unlock();
		                node->left->right->lock.unlock();
}

void leftlock(Node* node){
	node->parent->lock.lock();
	        node->lock.lock();
		        node->right->lock.lock();
			        node->right->left->lock.lock();
}
void leftunlock(Node* node){
	node->parent->lock.unlock();
	        node->lock.unlock();
		                node->right->lock.unlock();
				                        node->right->left->lock.unlock();
}

Node* insert(Node* node, int key,Node* parent)
{
       

       if (node == NULL){
	       if(c ==0){
	       c++;
	       return (rootNode(key));
	       
	       }
	       else{
	       Node* node1 = newNode(key,parent);
	       node1->lock.lock();
	       return node1;
	       }

       // return(node1);
}
 
    if (key < node->key){
	
	node->lock.lock();
        if(parent!=NULL){
	node->parent->lock.unlock();
	}
	node->left = insert(node->left, key, node);
    	node->left->parent->lock.unlock();
	node->left->lock.unlock();
    }
else if (key > node->key){
	    node->lock.lock();
	            if(parent!=NULL){
			            node->parent->lock.unlock();
				            }
        node->right = insert(node->right, key,node);
	node->right->parent->lock.unlock();
	node->right->lock.unlock();
    }
else  return node;

    node->height = 1 + max(height(node->left),
                        height(node->right));

int balance = getBalance(node);
     // Left Left Case
    
   if (balance > 1 && key < node->left->key)
{    rightlock(node);
	Node* n = rightRotate(node);
	leftunlock(n);
	return n;
}	
 
    // Right Right Case
    if (balance < -1 && key > node->right->key)
{ leftlock(node);
        Node* n = leftRotate(node);
 rightunlock(n);
 return n;
}
    // Left Right Case
    if (balance > 1 && key > node->left->key)
    {
	leftlock(node->left);
        node->left = leftRotate(node->left);
	rightunlock(node->left);
	rightlock(node);
        Node* n = rightRotate(node);
	leftunlock(n);
	return n;
    }
 
    // Right Left Case
    if (balance < -1 && key < node->right->key)
    {
	rightlock(node->right);
        node->right = rightRotate(node->right);
	leftunlock(node->right);
	leftlock(node);
        Node* n =  leftRotate(node);
	rightunlock(n);
	return n;
    }
 
    /* return the (unchanged) node pointer */
//node->lock.uplock();
    return node;
}

/*
void rightlock(Node* node){
	node->lock.lock();
	node->left.lock();
	node->left->right.lock();
}

void rightunlock(Node* node){
node->lock.unlock();
        node->left.unlock();
	        node->left->right.unlock();
}
void leftlock(Node* node){
	node->lock.lock();
	node->right->lock.lock();
	node->right->left->lock.lock();
}
void leftunlock(Node* node){
	node->lock.unlock();
	        node->right->lock.unlock();
		        node->right->left->lock.unlock();
}

*/

void preOrder(Node *root)
{
    if(root != NULL)
    {
        cout << root->key << " ";
        preOrder(root->left);
        preOrder(root->right);
    }
}

/*
int main()
{
//TEST(avltree::AVLTree, "Optimistic AVL Tree")
//int count = 0;
	Node *root = NULL;
     
  
    root = insert(root, 10,NULL);
    root = insert(root, 20,NULL);
    root = insert(root, 30,NULL);
    root = insert(root, 40,NULL);
    root = insert(root, 50,NULL);
    root = insert(root, 25,NULL);
     
       cout << "Preorder traversal of the "
            "constructed AVL tree is \n";
    preOrder(root);
     
    return 0;
}


 */



