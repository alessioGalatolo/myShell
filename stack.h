/*
 * an implementation of a stack of general object
 * gives an iterator for accessing data
 *
 *
 */


#ifndef MYSHELL_STACK_H
#define MYSHELL_STACK_H

#include <pthread.h>

typedef struct stack_node{
    unsigned int pos;
    struct stack_node* next;
    void* elem;
    size_t elem_size;
}stack_node;

typedef struct{
    stack_node* head;
    stack_node* iterator;
    size_t size;
    pthread_mutex_t mutex;
}stack;

stack* stack_init();
int stack_add(stack*, void*);
void* stack_getn(stack*, int);
int stack_reset_iterator(stack*);
void* stack_getnext(stack*);
int stack_destroy_wfree(stack*);
int stack_save_file(stack*, char*);
int stack_load_file(stack*, char*);

#endif //MYSHELL_STACK_H
