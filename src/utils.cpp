#include "tree.h"

#include <stdlib.h>
#include <vector>
#include <string>
#include <iostream>
using namespace std;
/******************
 * helper function
 ******************/

pthread_mutex_t show_tree_lock; // for print the whole tree

tree_node *create_dummy_node(void) {
	
	tree_node *new_node;
    	new_node = (tree_node *)malloc(sizeof(tree_node));
	new_node->value= INT32_MAX;
	new_node ->parent = NULL;
	new_node ->left_child = create_leaf_node();
	new_node ->right_child = create_leaf_node();
	new_node ->bf = 0;
	new_node ->is_leaf=false;
	new_node ->is_root=true;
	new_node ->flag = false;
	new_node ->marker = DEFAULT_MARKER;
	return new_node;
}
tree_node *create_node(int key) {
	
	tree_node *new_node;
    	new_node = (tree_node *)malloc(sizeof(tree_node));
	new_node -> value = key;
	new_node ->parent = nullptr;
	new_node ->left_child = nullptr;
	new_node ->right_child = nullptr;
	new_node ->is_leaf= true;
	new_node ->is_root= false;
	new_node ->bf = 0;
	new_node ->flag = false;
	new_node ->marker = DEFAULT_MARKER;
}

void show_tree(tree_node *root)
{
	pthread_mutex_lock(&show_tree_lock);
	dbg_printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	printf("[root] pointer: 0x%lx flag:%d\n", (unsigned long) root, (int) root->flag);

	tree_node *root_node = root->left_child;
	if (root_node->is_leaf)
	{
		printf("Empty Tree.\n");
		pthread_mutex_unlock(&show_tree_lock);
		return;
	}

	std::vector<tree_node *> frontier;
	frontier.clear();
	frontier.push_back(root_node);

	while (frontier.size() > 0)
	{
		tree_node *cur_node = frontier.back();
		tree_node *left_child = cur_node->left_child;
		tree_node *right_child = cur_node->right_child;

		printf("pointer: 0x%lx flag:%d marker: %d\n", (unsigned long) cur_node, (int) cur_node->flag, cur_node->marker);

		printf("(%d) bf\n", cur_node->bf);

		frontier.pop_back();
		if (left_child->is_leaf)
		{
			printf("    left null flag:%d pointer: 0x%lx\n", (int)left_child->flag, (unsigned long)left_child);
		}
		else
		{
			printf("    (%d) bf\n", left_child->bf);
			frontier.push_back(left_child);
		}

		if (right_child->is_leaf)
		{
			printf("    right null flag:%d pointer: 0x%lx\n", (int)right_child->flag, (unsigned long)right_child);
		}
		else
		{
			printf("    (%d) bf\n", right_child->bf);
			frontier.push_back(right_child);
		}
	}
	printf("\n++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	pthread_mutex_unlock(&show_tree_lock);
}


void inOrder(tree_node *root)  
{  
    if(root != NULL && root-> value !=0)  
    {  
//      inOrder(root->left_child);
	cout << root->value << " " << root-> bf << " " << root-> flag << endl ;  
	inOrder(root->left_child);
        inOrder(root->right_child);  
    }  
}  

/**
 * true if node is the root node, aka has a null parent
 */
bool is_root(tree_node *root, tree_node *node)
{
	if (node->parent == root)
	{
		return true;
	}
	else
	{
		return false;
	}
}

/**
 * create a leaf node, aka null node
 */
tree_node* create_leaf_node(void)
{
	tree_node *new_node;
	new_node = (tree_node *)malloc(sizeof(tree_node));

	new_node->value = 0;
	new_node->left_child = NULL;
	new_node->right_child = NULL;
	new_node->is_leaf = true;
	new_node->flag = false;
	new_node->marker = DEFAULT_MARKER;
	return new_node;
}

/**
 * replace the node with its child
 * this node has at most one non-nil child
 * return this child after modifying the relation ship
 */
tree_node *replace_parent(tree_node *root, tree_node *node)
{
	tree_node *child;
	if (node->left_child->is_leaf)
	{
		child = node->right_child;
		free_node(node->left_child);
	}
	else
	{
		child = node->left_child;
		free_node(node->right_child);
	}

	if (is_root(root, node))
	{
		child->parent = root;
		root->left_child = child;
		node->parent = NULL;
	}

	else if (is_left(node))
	{
		child->parent = node->parent;
		node->parent->left_child = child;
	}

	else
	{
		child->parent = node->parent;
		node->parent->right_child = child;
	}

	dbg_printf("[Remove] unlink complete.\n");
	return child;
}

/**
 * true if node is the left child of its parent node
 * false if it's right child
 */
bool is_left(tree_node *node)
{
	if (node->parent->is_leaf)
	{
		fprintf(stderr, "[ERROR] root node has no parent.\n");
	}

	tree_node *parent = node->parent;
    if (node == parent->left_child)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * get the uncle node of current node
 */
tree_node *get_uncle(tree_node *node)
{
    if (node->parent->is_leaf)
    {
        fprintf(stderr, "[ERROR] get_uncle node should be at least layer 3.\n");
        return NULL;
    }

    if (node->parent->parent->is_leaf)
    {
        fprintf(stderr, "[ERROR] get_uncle node should be at least layer 3.\n");
        return NULL;
    }

    tree_node *parent = node->parent;
    tree_node *grand_parent = parent->parent;
    if (parent == grand_parent->left_child)
    {
        return grand_parent->right_child;
    }
    else
    {
        return grand_parent->left_child;
    }
}

/**
 * free current node
 */
void free_node(tree_node *node)
{
    free(node);
}
