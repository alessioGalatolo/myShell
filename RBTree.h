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
  size_t key_length;
  int black; //boolean
}node;


typedef struct{
    node* root;
    pthread_mutex_t mutex;
}rb_tree;

rb_tree* tree_init();
int tree_insert(rb_tree*, void*, size_t);
static void insert_fixup(node*, node**);
static void right_rotation(node*, node**);
static void left_rotation(node*, node**);
void* tree_randsearch(rb_tree*, void*, size_t); //searches for the 'closest node'
static void* rec_randsearch(node*, void*, size_t, int);
static node* rec_search(node*, void*, size_t); //returns the node if found
int tree_print(rb_tree*);
static size_t min(size_t, size_t);
static int void_compare(void*, void*, size_t);

//coming next
//void tree_delete(node* z,node* *root);
//static void DeleteFixup(node* x,node* *root);
//static void RBTransplant(node* u,node* v,node* *root);
//static node* treemin(node* x); //min value


#endif //MYSHELL_RBTREE_H