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
}stack_node;

typedef struct{
    stack_node* head;
    stack_node* iterator;
    size_t size;
    pthread_mutex_t mutex;
}stack;

stack* stack_init();
int stack_add(stack*, void*);
int stack_reset_iterator(stack*);
void* stack_getnext(stack*);
int stack_destroy_wfree(stack* s);

#endif //MYSHELL_STACK_H
