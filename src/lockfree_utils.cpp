#include "tree.h"
#include<iostream>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <sys/types.h>

// thread local variables
thread_local vector<tree_node *> nodes_own_flag;
thread_local long thread_index;

void thread_index_init(long i)
{
    thread_index = i;
}

//clear local area
void clear_local_area(void)
{   
    if (nodes_own_flag.size() == 0) return;
    dbg_printf("[Flag] Clear\n");
    for (auto node : nodes_own_flag)
    {
        node->flag = false;
        dbg_printf("[Flag]      %d, 0x%lx, %d\n",
                   node->value, (unsigned long)node, (int)node->flag);
    }
    nodes_own_flag.clear();
}

bool is_in_local_area(tree_node *target_node)
{
    for (auto node : nodes_own_flag)
    {
        if (node == target_node) return true;        
    }
    
    return false;
}

bool setup_local_area_for_insert(tree_node *x)
{

    //std::cout << x->flag;
    tree_node *parent = x->parent;
    //std::cout << x->parent->flag;
    tree_node *uncle = NULL;

    if (parent == NULL) // as AVL tree is connectd, other nodes' parents cannot be null, only root's parent is null
        return true;

    bool expected = false;
    if (!parent->flag.compare_exchange_weak(expected, true))
    {
        dbg_printf("[FLAG] failed getting flag of 0x%lx\n", 
                   (unsigned long)parent);
        return false;
    }

    dbg_printf("[FLAG] get flag of 0x%lx\n", (unsigned long)parent);

    // abort when parent of x changes
    if (parent != x->parent)
    {
        dbg_printf("[FLAG] parent changed from %lu to 0x%lx\n", 
                   (unsigned long)parent, (unsigned long)parent);
        dbg_printf("[FLAG] release flag of 0x%lx\n", (unsigned long)parent);
        parent->flag = false;
        return false;
    }

    if (is_left(x))
    {
        uncle = x->parent->right_child;
    }
    else
    {
        uncle = x->parent->left_child;
    }

    expected = false;
    if (!uncle->flag.compare_exchange_weak(expected, true))
    {
        dbg_printf("[FLAG] failed getting flag of 0x%lx\n", (unsigned long)uncle);
        dbg_printf("[FLAG] release flag of 0x%lx\n", (unsigned long)x->parent);
        x->parent->flag = false;
        return false;
    }
#if 0
    if (uncle != x->parent->left_child && uncle != x->parent->right_child)
    {
	    dbg_printf("[FLAG] parent changed from %lu to 0x%lx\n", 
			    (unsigned long)parent, (unsigned long)uncle);
	    dbg_printf("[FLAG] release flag of 0x%lx\n", (unsigned long)uncle);
	    uncle->flag = false;
	    return false;
    }
#endif
    dbg_printf("[FLAG] get flag of 0x%lx\n", (unsigned long)uncle);

    // now the process has the flags of x, x's parent and x's uncle
    return true;
}

tree_node* up(tree_node *oldx, vector<tree_node *> &local_area)
{
	
   
	tree_node *oldp = oldx->parent;
    
    
    bool expected = false;

    tree_node *newx;
    tree_node *newp = NULL;
    tree_node *newgp = NULL; 
    tree_node *newuncle = NULL;
    newx = oldp;

    while (true && newx->parent != NULL)
    {
	
        newp = newx->parent;
        expected = false;
	
        if (!newp->flag.compare_exchange_weak(expected, true))
        {
	    
            continue;

        }
        newgp = newp->parent;
	
        if (newgp == NULL)
		
            break;
        expected = false;
        if (!newgp->flag.compare_exchange_weak(expected, true))
        {
            newp->flag = false;
            continue;
        }

        if (newp == newgp->left_child)
        {
	    
            newuncle = newgp->right_child;

        }
        else
        {
		
            newuncle = newgp->left_child;
        }

        expected = false;
        if (!newuncle->flag.compare_exchange_weak(expected, true))
        {
            newgp->flag = false;
            newp->flag = false;
		
            continue;
        }
        break;
    }
	
    local_area.push_back(newx);
    local_area.push_back(newp);
    local_area.push_back(newgp);
    local_area.push_back(newuncle);

    return newx;
}
vector<tree_node*> setup_local_area_for_delete(tree_node *y, tree_node *z)
{
    bool expect;

vector<tree_node*> local_area;
    tree_node *x = y->left_child;
    if (y->left_child->is_leaf)
        x = y->right_child;
    
    expect = false;
    if (!x->flag.compare_exchange_weak(expect, true)) return local_area;
    
    tree_node *yp = y->parent; 
    expect = false;
    if ((yp != z) && (!yp->flag.compare_exchange_weak(expect, true)))
    {
        x->flag = false;
        return local_area;
    }
    if (yp != y->parent) 
    {  
        x->flag = false; 
        if (yp!=z) yp->flag = false;
        return local_area;
    }
    tree_node *w = y->parent->left_child;
    if (is_left(y))
        w = y->parent->right_child;
    
    expect = false;
    if (!w->flag.compare_exchange_weak(expect, true))
    {
        x->flag = false;
        if (yp != z)
            yp->flag = false;
        return local_area;
    }

    tree_node *wlc, *wrc;
    if (!w->is_leaf)
    {
        wlc = w->left_child;
        wrc = w->right_child;

        expect = false;
        if (!wlc->flag.compare_exchange_weak(expect, true))
        {
            x->flag = false;
            w->flag = false;
            if (yp != z)
                yp->flag = false;
            return local_area;
        }
        expect = false;
        if (!wrc->flag.compare_exchange_weak(expect, true))
        {
            x->flag = false;
            w->flag = false;
            wlc->flag = false;
            if (yp != z)
                yp->flag = false;
            return local_area;
        }
    }
	
    local_area.push_back(x);
    local_area.push_back(w);
    local_area.push_back(yp);
    if (!w->is_leaf)
    {
        local_area.push_back(wlc);
        local_area.push_back(wrc);
        dbg_printf("[Flag] local area: %d %d %d %d %d\n",
                   x->value, w->value, yp->value, wlc->value, wrc->value);
    }
    else
    {
        dbg_printf("[Flag] local area: %d %d %d\n",
                   x->value, w->value, yp->value);
    }

	
    return local_area;
}
tree_node *par_find(tree_node *root, int value)
{
    bool expect;
    tree_node *root_node;
restart:
    do {

        root_node = root->left_child;

        expect = false;

    } while (!root_node->flag.compare_exchange_weak(expect, true));
    
    tree_node *y = root_node;
    tree_node *z = NULL;

    while (!y->is_leaf)
    {
        z = y; 
        if (value == y->value)
            return y; 
        else if (value > y->value)
            y = y->right_child;
        else
            y = y->left_child;
        
        expect = false;
	
        if (!y->flag.compare_exchange_weak(expect, true))
        {
            z->flag = false; 
            usleep(100);
            goto restart;
        }
        if (!y->is_leaf)
            z->flag = false;
    }
    
    dbg_printf("[WARNING] node with value %d not found.\n", value);
    return NULL;
}

tree_node *par_find_successor(tree_node *delete_node)
{
    bool expect;
    

    tree_node *y = delete_node->right_child;
    tree_node *z = NULL;
	
    while (!y->left_child->is_leaf)
    {
        z = y; 
        y = y->left_child;

        expect = false;
        if (!y->flag.compare_exchange_weak(expect, true))
        {
            z->flag = false; 
            return NULL; 
        }
        
        z->flag = false;
    }
	    
    return y; 
}
