/*
 * Created by Alessio on 3/24/20.
 *
 * A library implementing a linked list.
 */

#ifndef DSOPP_SYNTHESIS_LINKEDLIST_H
#define DSOPP_SYNTHESIS_LINKEDLIST_H

#include <stddef.h>

typedef struct _node{
    struct _node* next;
    struct _node* parent;
    void* obj;
    size_t size; //size of the object
}node_t;

//non recursive struct
typedef struct{
    node_t* head;
    node_t* tail;
    size_t length;
}llist_t;

llist_t* llist_create(); //creates the list
int llist_headinsert(llist_t*, void*, size_t); //adds a product to the list in the head
int llist_tailinsert(llist_t*, void*, size_t); //adds a product to the list in the tail
void* llist_getlast(llist_t*, size_t*);
void** llist_as_array(llist_t*, size_t**);
size_t llist_length(llist_t*); //length of the list
void llist_destroy(llist_t*); //frees the memory

#endif //DSOPP_SYNTHESIS_LINKEDLIST_H
