//
// Created by Alessio on 7/21/2019.
//

#include <string.h>
#include <stdlib.h>
#include "cmd_storage.h"
#include "RBTree.h"
#include "stack.h"

rb_tree* cmd_tree = NULL;
stack* cmd_stack = NULL;

static void check_initialization(){
    if(cmd_tree == NULL)
        cmd_tree = tree_init();
    if(cmd_stack == NULL)
        cmd_stack = stack_init();
}

int store_command(char** args) {
    check_initialization();
    size_t total_length = 0;
    int i = 0;
    while (args[i] != NULL) {
        total_length += strlen(args[i]) + 1;
        i++;
    }
    char* cmd = malloc(sizeof(total_length));
    if(cmd == NULL)
        return 0;
    strcpy(cmd ,args[0]);
    strcat(cmd, " ");
    i = 1;
    while(args[i] != NULL){
        strcat(cmd, args[i]);
        strcat(cmd, " ");
        i++;
    }

    int outcome = 0;
    outcome += stack_add(cmd_stack, cmd);
    outcome += tree_insert(cmd_tree, cmd, strlen(cmd));

    return outcome == 2? 1: 0;
}

char* search_command(char* cmd){
    check_initialization();
    if(cmd == NULL)
        return NULL;
    return tree_randsearch(cmd_tree, cmd, strlen(cmd));
}