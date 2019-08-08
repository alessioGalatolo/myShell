//
// Created by Alessio on 7/21/2019.
//

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include "cmd_storage.h"
#include "RBTree.h"
#include "stack.h"

rb_tree* cmd_tree = NULL;
stack* cmd_stack = NULL;

pthread_t autosave_thread = 0;
int autosave_terminate = 0;

static void check_initialization(){
    if(cmd_tree == NULL)
        cmd_tree = tree_init();
    if(cmd_stack == NULL)
        cmd_stack = stack_init();
}

static void* store_thread_fun(void* arg){
    tree_save_file(cmd_tree, (char*) arg);
    stack_save_file(cmd_stack, (char*) arg);
    return (void*) 0;
}

void autosave_tofile(int on){
    if(on && autosave_thread == 0){
        autosave_terminate = 0;
        //launch thread
    }else if(!on){
        autosave_terminate = 1;
    }
}

static int store_tofile(){
    char* path = "/lib/var/myShell/saved_commands";
    //check folder existence
    char* dir = "/lib/var/myshell";
    if(access(dir, F_OK) == -1){//not found
        mkdir(dir, 0700);
    }

    pthread_t id;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    pthread_create(&id, &attr, store_thread_fun, path);
    return 1;
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
    strcpy(cmd, args[0]);
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