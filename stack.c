//TODO: merge into linked list library

#include "stack.h"
#include "utils.h"
#include <stdlib.h>
#include <stdio.h>


    //TODO: check IO return values, add fclose
    //TODO: add mutex lock to load/save from file

stack* stack_init(){
    stack* s = malloc(sizeof(stack));
    s -> head = NULL;
    pthread_mutex_init(&(s -> mutex), NULL);
    s -> size = 0;
    s -> iterator = NULL;
    return s;
}

int stack_add(stack* s, void* obj){
    NULL_CHECK(s);
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    stack_node* oldhead = s -> head;
    if((s->head = malloc(sizeof(stack_node))) == NULL){
        s->head = oldhead;
        THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
        return 0;
    }
    s->head->elem = obj;
    s->head->next = oldhead;
    if(oldhead == NULL)
        s->head->pos = 1;
    else
        s->head->pos = (oldhead -> pos) + 1;
    (s->size)++;
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    return 1;
}

//get n-th element
void* stack_getn(stack* s, int n){
    THREAD_CHECK(pthread_mutex_lock(&s -> mutex));
    stack_node* node = s -> head;
    int i = 0;
    while(i < n - 1 && node != NULL){
        node = node -> next;
        i++;
    }
    THREAD_CHECK(pthread_mutex_unlock(&s -> mutex));
    return node -> elem;
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

void stack_rec_destroy(stack_node* s){
    if(s != NULL){
        stack_rec_destroy(s -> next);
        free(s -> elem);
        free(s);
    }
}

void stack_rec_write(stack_node* s, FILE* file){
    if(s != NULL) {
        stack_rec_write(s->next, file);
        fwrite(&s -> elem_size, sizeof(size_t), 1, file);
        fwrite(s -> elem, 1, s -> elem_size, file);
    }
}

int stack_save_file(stack* s, char* path){
    FILE* file = fopen(path, "w");
    if(file == NULL)
        return 0;
    stack_rec_write(s->head, file);
    size_t zero = 0;
    fwrite(&zero, sizeof(size_t), 1, file);
    FCLOSE(file);
    return 1;
}

int stack_load_file(stack* s, char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL)
        return 0;
    if((s -> head = malloc(sizeof(stack_node))) == NULL)
        return 0;
    fread(&s -> head -> elem_size, sizeof(size_t), 1, file);
    if(s -> head -> elem_size == 0)
        return 1;
    if((s -> head -> elem = malloc(s -> head -> elem_size)) == NULL)
        return 0;
    fread(&s -> head -> elem, 1, s -> head -> elem_size, file);
    size_t cur_size = 0;
    fread(&cur_size, sizeof(size_t), 1, file);
    stack_node* cur_node = s -> head;
    while(cur_size != 0){
        if((cur_node -> next = malloc(cur_size)) == NULL)
            return 0;
        fread(cur_node -> next, 1, cur_size, file);
        cur_node = cur_node -> next;
        fread(&cur_size, sizeof(size_t), 1, file);
    }
    return 1;
}

int stack_destroy_wfree(stack* s){
    if(s == NULL)
        return 1;
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    stack_rec_destroy(s -> head);
    THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
    THREAD_CHECK(pthread_mutex_destroy(&(s -> mutex)));
    return 1;
}

