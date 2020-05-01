/*
 * Created by Alessio on 3/24/20.
 */
#include "linkedlist.h"
#include "utils.h"

/**
 * Creates the linked list
 * @return A pointer to the linked list
 */
llist_t* llist_create(){
    llist_t* list;
    MALLOC(list, sizeof(llist_t), ;);
    list->head = NULL;
    list->tail = NULL;
    list->length = 0;
    return list;
}

int llist_headinsert(llist_t* list, void* obj, size_t size){
    NULL_CHECK(list);
    node_t* node = list->head;
    MALLOC(list->head, sizeof(node_t), ;);
    list->head->next = node;
    list->head->parent = NULL;
    list->head->obj = obj;
    list->head->size = size;
    list->length++;
    if(node)
        node->parent = list->head;
    else //head was null, so is tail
        list->tail = list->head;
    return 1;
}

int llist_tailinsert(llist_t* list, void* obj, size_t size){
    NULL_CHECK(list);
    if(list->tail == NULL)
        return llist_headinsert(list, obj, size);

    node_t* previous_node = list->tail;
    MALLOC(list->tail->next, sizeof(node_t), ;);
    list->tail->next->parent = list->tail;
    list->tail = list->tail->next;
    list->tail->next = NULL;
    list->tail->obj = obj;
    list->tail->size = size;
    list->length++;
    return 1;
}

void* llist_getlast(llist_t* list, size_t* size){
    NULL_CHECK(list);
    if(list->tail) {
        if (size)
            *size = list->tail->size;
        return list->tail->obj;
    }
    return 0;
}

//TODO: should also return array of sizes
void** llist_as_array(llist_t* list, size_t** sizes){
    NULL_CHECK(list);
    void** array;
    MALLOC(array, sizeof(void*) * (list->length + 1), ;);
    node_t* node = list->head;
    int i = 0;
    for(; i < list->length; i++){
        array[i] = node->obj;
        node = node->next;
    }
    array[i] = NULL;
    return array;
}

/**
 * @return The length of a linked list
 */
size_t llist_length(llist_t* list){
    NULL_CHECK(list);
    return list->length;
}

/**
 * Frees all the memory used by the list
 */
void llist_destroy(llist_t* list){
    if(list != NULL){
        while(list->head != NULL){
            node_t* to_free = list->head;
            list->head = list->head->next;
            list->length--;
            free(to_free);
        }
        free(list);
    }
}