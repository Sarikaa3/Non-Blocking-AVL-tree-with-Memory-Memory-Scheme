#include "tree.h"
#include<iostream>

#include <stdlib.h>

// init

int main(int argc, char const *argv[])
{
    tree_node *root = avl_init();
    
    avl_insert(root, 10);
inOrder(root);
    avl_insert(root, 20);
inOrder(root);
    avl_insert(root, 30);
inOrder(root);
   
    avl_insert(root, 40);
inOrder(root);
 avl_insert(root, 50);
inOrder(root);
 avl_insert(root,25);
inOrder(root);
show_tree(root);

 //  inOrder(root);
  //  rb_insert(root, 7);
  //  rb_insert(root, 6);
  // inOrder(root);
   // show_tree(root);
  avl_remove(root, 20);
inOrder(root);
    //show_tree(root);
   // rb_remove(root, 7);
  //  show_tree(root);
   // rb_remove(root, 3);
    //show_tree(root);
    return 0;
}
