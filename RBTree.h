//
// Created by Alessio on 7/19/2019.
//

#ifndef MYSHELL_RBTREE_H
#define MYSHELL_RBTREE_H

#include <pthread.h>

typedef struct node{
  struct node* right;
  struct node* left;
  struct node* parent;
  void* key;
  int black; //boolean
}node;


typedef struct{
    node* root;
    pthread_mutex_t mutex;
    int (*compare) (void*, void*); //compare function
}rb_tree;

rb_tree* tree_init(int (*compare) (void*, void*));
int tree_insert(rb_tree *, void* obj);
static void insert_fixup(node* z,node* *root);
static void right_rotation(node* x,node* *root);
static void left_rotation(node* x,node* *root);
void* tree_randsearch(rb_tree*, void* obj); //searches for the 'closest node'
static node* rec_search(node* x, void* obj, int (*compare)(void*, void*)); //returns the node if found
int tree_print(rb_tree*);

//coming next
//void tree_delete(node* z,node* *root);
//static void DeleteFixup(node* x,node* *root);
//static void RBTransplant(node* u,node* v,node* *root);
//static node* treemin(node* x); //min value


#endif //MYSHELL_RBTREE_H