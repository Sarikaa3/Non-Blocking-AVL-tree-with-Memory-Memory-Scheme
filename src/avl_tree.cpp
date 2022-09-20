#include "tree.h"
#include<atomic>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <vector>
#include <iomanip>
#include <stdlib.h>

extern thread_local long thread_index;
int count = 0;

tree_node *avl_init(void)
{
	//tree_node *dummy1 = create_dummy_node();
	//tree_node *dummy2 = create_dummy_node();
	//tree_node *dummy3 = create_dummy_node();
	//tree_node *dummy4 = create_dummy_node();
	//tree_node *dummy5 = create_dummy_node();
	tree_node *dummy_sibling = NULL;
	tree_node *root = create_dummy_node();

	//dummy_sibling->parent = root;
	//root->parent = dummy5;
	//dummy5->parent = dummy4;
	//dummy4->parent = dummy3;
	//dummy3->parent = dummy2;
	//dummy2->parent = dummy1;

	//free_node(dummy1->left_child);
	//dummy1->left_child = dummy2;
	//free_node(dummy2->left_child);
	//dummy2->left_child = dummy3;
	//free_node(dummy3->left_child);
	//dummy3->left_child = dummy4;
	//free_node(dummy4->left_child);
	//dummy4->left_child = dummy5;
	//free_node(dummy5->left_child);
	//dummy5->left_child = root;
	free_node(root->right_child);
	root->right_child = NULL;
	return root;
}

void leftRotate(tree_node *root, tree_node* x) {
	tree_node* y = x->right_child;
	x->right_child = y->left_child;
	if (y->left_child != nullptr) {
		y->left_child->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == nullptr) {
		root = y;
	} else if (x == x->parent->left_child) {
		x->parent->left_child = y;
	} else {
		x->parent->right_child = y;
	}
	y->left_child = x;
	x->parent = y;

	//balance factor
	x->bf = x->bf - 1 - max(0, y->bf);
	y->bf = y->bf - 1 + min(0, x->bf);
}

void rightRotate(tree_node* root, tree_node *x) {
	tree_node *y = x->left_child ;
	x->left_child  = y->right_child ;
	if (y->right_child  != nullptr) {
		y->right_child ->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == nullptr) {
		root = y;
	} else if (x == x->parent->right_child ) {
		x->parent->right_child  = y;
	} else {
		x->parent->left_child  = y;
	}
	y->right_child  = x;
	x->parent = y;

	// again the balance factor
	x->bf = x->bf + 1 - min(0, y->bf);
	y->bf = y->bf + 1 + max(0, x->bf);
}

void tree_insert(tree_node *root, tree_node *new_node)
{
	int value = new_node->value;
	
	bool expected = false;
	
	//change root node to true, in case if the tree is empty
	while (!root->flag.compare_exchange_weak(expected, true)); 
	//printf("[FLAG] get flag of 0x%lx\n", (unsigned long)root);

	//std::cout << root->left_child->is_leaf;
	if (root->left_child->is_leaf)
	{
		
		free_node(root->left_child);
		new_node->flag = true;
		//dbg_printf("[FLAG] set flag of 0x%lx\n", (unsigned long)new_node);
		root->left_child = new_node;
		new_node->parent = root;
		

		//printf("[Insert] new node with value (%d)\n", value);
		//printf("[FLAG] release flag of 0x%lx\n", (unsigned long)root);
		root->flag = false;
		return;
	}
	
	//printf("[FLAG] release flag of 0x%lx\n", (unsigned long)root);
	root->flag = false;

	restart:
	
	tree_node *z = NULL;
	tree_node *curr_node = root->left_child;
	
	expected = false;
	if (!curr_node->flag.compare_exchange_strong(expected, true))
	{
		//printf("[FLAG] failed getting flag of 0x%lx\n", (unsigned long)curr_node);

		goto restart;
	}
	
	//printf("[FLAG] get flag of 0x%lx\n", (unsigned long)curr_node);
	while (!curr_node->is_leaf)
	{
		z = curr_node;
		if (value > curr_node->value) /* go right */
		{
			curr_node = curr_node->right_child;
		}
		else /* go left */
		{
			curr_node = curr_node->left_child;
		}

		expected = false;
		if (!curr_node->flag.compare_exchange_weak(expected, true))
		{
			//printf("[FLAG] failed getting flag of 0x%lx\n", (unsigned long)curr_node);
			//printf("[FLAG] release flag of 0x%lx\n", (unsigned long)z);
			z->flag = false;// release z's flag

			goto restart;
		}

		//printf("[FLAG] get flag of 0x%lx\n", (unsigned long)curr_node);

		if (!curr_node->is_leaf)
		{
			// release ancestor flag
			//printf("[FLAG] release flag of 0x%lx\n", (unsigned long)z);
			z->flag = false;
		}
	}
	
	new_node->flag = true;
	if (!setup_local_area_for_insert(z))
	{
		curr_node->flag = false;
		//printf("[FLAG] release flag of %lu and %lu\n", (unsigned long)z, (unsigned long)curr_node);
	z->flag = false;
		goto restart;
	}

	// now the local area has been setup
	// insert the node and set the parent as z
	new_node->parent = z;
	
	if (value <= z->value)
	{
		free(z->left_child);
		z->left_child = new_node;
	}
	else
	{
		free(z->right_child);
		z->right_child = new_node;
	}
}
void avl_insert(tree_node *root, int value)
{

	clear_local_area();

	tree_node *new_node;
	new_node = (tree_node *)malloc(sizeof(tree_node));
	new_node->value=value;
	new_node->left_child = create_leaf_node();
	new_node->right_child = create_leaf_node();;
	new_node->flag = false;
	new_node->is_leaf = false;
	new_node->parent=NULL;
	new_node->bf=0;
	new_node->marker = DEFAULT_MARKER;
	
	

	tree_insert(root, new_node); // normal insert
	
	tree_node *curr_node = new_node;
	

	tree_node *parent, *uncle  = NULL, *grandparent = NULL;

	parent = curr_node->parent;

	vector<tree_node *> local_area = {curr_node, parent};
	
	cout << parent->is_root << "\n";
	if (!parent->is_root) //if the parent is the parent of the root
	{
		
		grandparent = parent->parent; 
		local_area.push_back(grandparent);
	}

	if (grandparent != NULL) //same case as before
	{
		
		if (grandparent->left_child == parent)
		{
			uncle = grandparent->right_child;
		}
		else
		{
			uncle = grandparent->left_child;
		}
		local_area.push_back(uncle);
	}
	//as we are sure that the local area is set, we are pushing the nodes to the locally created vector
	//local_area.push_back(uncle);
	//local_area.push_back(grandparent);	
	
	//curr_node = move_inserter_up(curr_node, local_area);
	/*for (auto curr_node:local_area)
		        cout << curr_node->value << " ";
			cout << "\n";*/
	cout << curr_node->parent->value << "\n";
	updateBalance(root, curr_node, local_area);	
	/*for (auto curr_node:local_area)
	                          cout << curr_node->value << " ";
	                                                  cout << "\n";
	//clear local area*/
	
	cout << value << "inserted" << "\n";
	
	inOrder(root);	
	
	for (auto curr_node:local_area)
	{
		cout << curr_node->flag << "\n";
		if (curr_node != NULL)
		{				//printf("[FLAG] release flag of %lu\n", (unsigned long)curr_node);
			curr_node->flag = false;
		}
		
			}

}

void updateBalance(tree_node * root, tree_node* curr_node, vector< tree_node*> local_area) {

	//cout << curr_node->value << "\n";
	if (curr_node->bf < -1 || curr_node->bf > 1) {
		
		rebalance(root, curr_node);
		for (auto curr_node:local_area)
			{
				if (curr_node != NULL)
				{
					//printf("[FLAG] release flag of %lu\n", (unsigned long)curr_node);
					curr_node->flag = false;
				}
			}

			local_area.clear();
		return;
	}

	if (curr_node->parent != NULL) {
		if (curr_node == curr_node->parent->left_child) {
			curr_node->parent->bf -= 1;

		} 

		if (curr_node == curr_node->parent->right_child) {
			curr_node->parent->bf += 1;

		}

		if ( curr_node->parent->bf != 0) {
			 
			for (auto curr_node:local_area)
			{
				if (curr_node != NULL)
				{
					//printf("[FLAG] release flag of %lu\n", (unsigned long)curr_node);
					curr_node->flag = false;
				}
			}

			local_area.clear();

			//cout << curr_node->parent->bf;
			curr_node = up(curr_node,local_area);
		
			updateBalance(root, curr_node, local_area);


		}
	
	}
	for (auto curr_node:local_area)
			{
				if (curr_node != NULL)
				{
					//printf("[FLAG] release flag of %lu\n", (unsigned long)curr_node);
					curr_node->flag = false;
				}
			}

			local_area.clear();

}
void rebalance(tree_node* root, tree_node* node) {
	if (node->bf > 0) {
		if (node->right_child->bf < 0) {
			rightRotate(root, node->right_child);
			leftRotate(root, node);
		} else {
			leftRotate(root, node);
		}
	} else if (node->bf < 0) {
		if (node->left_child->bf > 0) {
			leftRotate(root, node->left_child);
			rightRotate(root, node);
		} else {
			rightRotate(root, node);
		}
	}
}

void avl_remove(tree_node *root, int value)
{


    clear_local_area();
     vector<tree_node *> local_area;
restart:
	
    tree_node *z = par_find(root, value);
	
    tree_node *y; 
    if (z == NULL)
        return;

    if (z->left_child->is_leaf || z->right_child->is_leaf)
        y = z;
    else
        y = par_find_successor(z);
    
	
    if (y == NULL)
    {
        z->flag = false;
	
        goto restart;
    }
    
	local_area = setup_local_area_for_delete(y, z);
    
	
    if (local_area.size()==0)
    {
        // release flags
        y->flag = false;
        if (y != z) z->flag = false;
        goto restart; // deletion failed, try again
    }
    
    tree_node *replace_node = replace_parent(root, y);

    
    if (y != z)
        z->value = y->value;
    
   
    if (!is_in_local_area(z))
    {
        z->flag = false;
           }  
	
	updateBalance(root, replace_node, local_area);
	for (auto curr_node:local_area)
			{
				if (curr_node != NULL)
				{
					//printf("[FLAG] release flag of %lu\n", (unsigned long)curr_node);
					curr_node->flag = false;
				}
			}

			local_area.clear();
    
	cout << value << "deleted" << endl;   
	
    clear_local_area();
    
    free_node(y);
}


