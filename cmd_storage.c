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

#define CMD_STORE_PATH "/var/lib/myShell/saved_commands"
#define CMD_STORE_DIR "/var/lib/myShell"

rb_tree* cmd_tree = NULL;
stack* cmd_stack = NULL;

pthread_t autosave_thread = 0;
int autosave_terminate = 0;

//to be used when scrolling between cmds
int incomplete_cmd = 0;//TODO



static void check_initialization(){
    if(cmd_tree == NULL) {
        cmd_tree = tree_init();
        tree_load_file(cmd_tree, CMD_STORE_PATH);
    }
    if(cmd_stack == NULL) {
        cmd_stack = stack_init();
        stack_load_file(cmd_stack, CMD_STORE_PATH);
    }
}

static void* store_thread_fun(void* arg){
    //TODO


    return (void*) 0;
}

void autosave_tofile(int on){
    if(on && autosave_thread == 0){
        autosave_terminate = 0;
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        pthread_create(&autosave_thread, &attr, store_thread_fun, NULL);
        //launch thread
    }else if(!on){
        autosave_terminate = 1;
    }
}

static int store_tofile(){
    //checks folder existence
    if(access(CMD_STORE_DIR, F_OK) == -1){//not found
        mkdir(CMD_STORE_DIR, 0700);
    }

    tree_save_file(cmd_tree, CMD_STORE_PATH);
    stack_save_file(cmd_stack, CMD_STORE_PATH);
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

char* cmd_previous(char* current){
    incomplete_cmd = 1;
    //TODO: get latest
}

void cmd_exit(){
    autosave_terminate = 1;
    store_tofile();
    stack_destroy_wfree(cmd_stack);
    //TODO: tree_destroy_wfree(cmd_tree);
}