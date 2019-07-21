

#include "stack.h"
#include <stdlib.h>

#define THREAD_CHECK(x)\
    if((x) != 0){return 0;}

stack* stack_init(){
    stack* s = malloc(sizeof(stack));
    s -> head = NULL;
    pthread_mutex_init(&(s -> mutex), NULL);
    s -> size = 0;
    s -> iterator = NULL;
    return s;
}

int stack_add(stack* s, void* obj){
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    stack_node* oldhead = s -> head;
    if((s -> head = malloc(sizeof(stack_node))) == NULL){
        s -> head = oldhead;
        THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
        return 0;
    }
    s -> head -> elem = obj;
    s -> head -> next = oldhead;
    if(oldhead == NULL)
        s -> head -> pos = 1;
    else
        s -> head -> pos = (oldhead -> pos) + 1;
    (s -> size)++;
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    return 1;
}


int stack_reset_iterator(stack* s){
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    s -> iterator = NULL;
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    return 1;
}


void* stack_getnext(stack* s){
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    if(s -> iterator == NULL){
        s -> iterator = s -> head;
        void* obj = s -> iterator -> elem;
        THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
        return obj;
    }
    s -> iterator = s -> iterator -> next;
    if(s -> iterator == NULL) {
        THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
        return NULL;
    }
    void* obj = s -> iterator -> elem;
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    return obj;
}

void rec_destroy(stack_node* s){
    if(s != NULL){
        rec_destroy(s -> next);
        free(s -> elem);
        free(s);
    }
}


int stack_save_file(stack*, char*);
int stack_load_file(stack*, char*);

int stack_destroy_wfree(stack* s){
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    rec_destroy(s -> head);
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    THREAD_CHECK(pthread_mutex_destroy(&(s -> mutex)));
    return 1;
}

