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

int stack_add(stack* s, void* obj, size_t size){
    NULL_CHECK(s);
    THREAD_CHECK(pthread_mutex_lock(&(s -> mutex)));
    stack_node* oldhead = s -> head;
    if((s->head = malloc(sizeof(stack_node))) == NULL){
        s->head = oldhead;
        THREAD_CHECK(pthread_mutex_unlock(&(s -> mutex)));
        return 0;
    }
    s->head->elem = obj;
    s->head->elem_size = size;
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
        fwrite(&s->elem_size, sizeof(size_t), 1, file);
        fwrite(s->elem, 1, s->elem_size, file);
    }
}

int stack_save_file(stack* s, char* path){
    FILE* file = fopen(path, "w");
    if(file == NULL)
        return 0;
    stack_rec_write(s->head, file);
    size_t zero = 0; //suggests last element
    fwrite(&zero, sizeof(size_t), 1, file);
    FCLOSE(file);
    return 1;
}

//TODO: not thread safe
void stack_print(stack* s){
    if(s){
        printf("Stack loaded is: \n");
        stack_node* node = s->head;
        while(node){
            printf("%s\n", node->elem);
            node = node->next;
        }
    }
}

int stack_load_file(stack* mStack, char* path){
    FILE* file = fopen(path, "r");
    if(file == NULL)
        return 0;
    size_t size2read;
    void* elem2read;
    fread(&size2read, sizeof(size_t), 1, file);
    while(size2read != 0){
        MALLOC(elem2read, size2read, ;);
        fread(elem2read, 1, size2read, file);
        stack_add(mStack, elem2read, size2read);
        fread(&size2read, sizeof(size_t), 1, file);
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



