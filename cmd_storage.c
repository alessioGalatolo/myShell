//
// Created by Alessio on 7/21/2019.
//

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/stat.h>
#include <stdio.h>
#include <pwd.h>
#include <sys/types.h>
#include "cmd_storage.h"
#include "RBTree.h"
#include "stack.h"
#include "utils.h"


char* cmd_store_dir = NULL; //the path to the directory where to store files
char* cmd_stack_path = NULL;
char* cmd_tree_path = NULL;

rb_tree* cmd_tree = NULL;
stack* cmd_stack = NULL;

pthread_t autosave_thread = 0;
int autosave_terminate = 0;

//to be used when scrolling between cmds
int incomplete_cmd = 0;//TODO


static int check_initialization(){
    if(!cmd_stack_path){
        struct passwd *pw = getpwuid(getuid());
        const char *homedir = pw->pw_dir;
        char* extension = "/.myShell"; //TODO: replace with yash
        MALLOC(cmd_store_dir, sizeof(char) * (strlen(homedir) + strlen(extension) + 1), ;);
        strcpy(cmd_store_dir, homedir);
        strcat(cmd_store_dir, extension);
        MALLOC(cmd_stack_path, sizeof(char) * strlen(cmd_store_dir) + 16, FREE(cmd_store_dir));
        strcpy(cmd_stack_path, cmd_store_dir);
        strcat(cmd_stack_path, "/commands_stack");
        MALLOC(cmd_tree_path, sizeof(char) * strlen(cmd_store_dir) + 15, FREE(cmd_store_dir); FREE(cmd_stack_path));
        strcpy(cmd_tree_path, cmd_store_dir);
        strcat(cmd_tree_path, "/commands_tree");
    }
    if(cmd_tree == NULL) {
        cmd_tree = tree_init();
        tree_load_file(cmd_tree, cmd_tree_path);
    }
    if(cmd_stack == NULL) {
        cmd_stack = stack_init();
        stack_load_file(cmd_stack, cmd_stack_path);
    }
    return 1;
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
    if(cmd_store_dir) {
        //checks folder existence
        if (access(cmd_store_dir, F_OK) == -1) {//not found
            if (mkdir(cmd_store_dir, 0777) == -1)
                perror("mkdir error during store tofile");
        }

        if (cmd_tree != NULL)
            tree_save_file(cmd_tree, cmd_tree_path);
        if (cmd_stack != NULL)
            stack_save_file(cmd_stack, cmd_stack_path);
    }
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
    char* cmd; //stores command with arguments
    MALLOC(cmd, sizeof(char) * total_length + 1, ;);
    strcpy(cmd, args[0]);
    strcat(cmd, " ");
    i = 1;
    while(args[i] != NULL){
        strcat(cmd, args[i]);
        strcat(cmd, " ");
        i++;
    }
    //TODO: replace last space with \0?

    int outcome = 0;
    outcome += stack_add(cmd_stack, cmd, (strlen(cmd) + 1) * sizeof(char));
    outcome += tree_insert(cmd_tree, cmd, (strlen(cmd) + 1) * sizeof(char));

    return outcome == 2? 1: 0;
}

char* search_command(char* cmd){
    check_initialization();
    if(cmd == NULL)
        return NULL;
    //TODO: make return a list of string
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
    FREE(cmd_stack_path);
    FREE(cmd_tree_path);
    FREE(cmd_store_dir);
}