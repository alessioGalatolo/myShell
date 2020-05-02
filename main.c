#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include "linkedlist.h"
#include "cmd_storage.h"

#define COMMAND_BASE_LENGTH 100
#define ARG_MAX_LENGTH 20
#define COMPLETE_FILE "./complete_file~"

#define COMMAND_LIST_COMMAND find /bin -executable -maxdepth 1

//max attempts for malloc, realloc, ...
#define MAX_ATTEMPTS 3

//utility functions / macros
#define FREE(x)\
    if((x) != NULL){free(x); x = NULL;}
void *Malloc(size_t size);
void *Realloc(void *buf, size_t size);

//keep attributes of terminal
struct termios saved_attributes;

char *read_command(ssize_t *readen);
void set_input_mode();
char* tab_complete(char *input);
int run_command(char **args);
int exec_local_cmd(char* cmd, char** args);


//TODO change ARG_MAX_LENGTH
int main(){
    ssize_t input_length;
    char *input;
    char *command; //stores the typed command

    set_input_mode();

    int go_on = 1; //controls exit
    do{
        char *save_ptr; //used for tokenizer memory
        llist_t* arg_list = llist_create();
        char* cwd = getcwd(NULL, 0);
        printf("%s: ", cwd); //TODO: replace printf
        FREE(cwd);

        //read and parse input
        input = read_command(&input_length);
        command = strtok_r(input, " ", &save_ptr);
        if(command != NULL) {
            char *current_arg = strtok_r(NULL, " ", &save_ptr);
            while (current_arg != NULL) {
                llist_tailinsert(arg_list, current_arg, 0);
                current_arg = strtok_r(NULL, " ", &save_ptr);
            }

            //check termination
            go_on = strncmp("exit", command, sizeof(char) * 4);
            if (go_on != 0) {
                if (strcmp(command, "sudo") == 0) {
                    llist_headinsert(arg_list, "-S", 0); //Todo: may be unnecessary
                }
                llist_headinsert(arg_list, command, 0);
                char **args = (char **) llist_as_array(arg_list, NULL);
                store_command(args); //TODO: run in separate thread
                run_command(args);
                FREE(args);
            } else {
                cmd_exit();
            }
        }
        llist_destroy(arg_list);
        FREE(input);
    }while(go_on);

    return 0;
}

/**
 * Tries to run the command with the given arguments
 * @return 1 in case of success, 0 otherwise
 */
int run_command(char **args) {
    char* command = args[0];
    if(exec_local_cmd(command, args)) //look in local commands
        return 1;
    int pid = fork();
    if(pid < 0){
        perror("fork error");
        return 0;
    }else if(pid == 0){
        //TODO: change length
        char cmd[COMMAND_BASE_LENGTH + 5];
        sprintf(cmd, "/bin/");
        strncat(cmd, command, COMMAND_BASE_LENGTH + 5);
        if(strcmp(command, "sudo") == 0){

        }
        execvp(cmd, args);
        if(errno == ENOENT) {
            sprintf(cmd, "/usr/bin/");
            strncat(cmd, command, COMMAND_BASE_LENGTH + 5);
            execvp(cmd, args);
            printf("\'%s\' command not found\n", command);
            exit(1);
        } else
            perror("Exec");
        return 0;
    }else {
        waitpid(pid, NULL, 0);
    }
    return 1;
}

/**
 * Reads from input till new line
 * @param readen Pointer to size_t to store the number of char read
 * @return The string read
 */
char *read_command(ssize_t *readen) {
    size_t input_length = COMMAND_BASE_LENGTH;
    char* input = Malloc(input_length);
    *readen = 0;
    int terminate = 0;
    do{

        size_t read_n = 0;
        if(*readen > input_length - COMMAND_BASE_LENGTH / 5){
            input = Realloc(input, sizeof(char) * (input_length + COMMAND_BASE_LENGTH));
            input_length += COMMAND_BASE_LENGTH;
        }

        read_n = fread(input + *readen, sizeof(char), 1, stdin);
        if(read_n == 0){
            //TODO: handle error
        }
        switch(input[*readen]) {
            case '\0':
                terminate = 1;
                break;
            case '\n':
                putchar('\n');
                terminate = 1;
                break;
            case 127: //delete key
                if (*readen > 0) {
                    input[*readen] = '\0';
                    putchar(8);
                    putchar(' ');
                    putchar(8);
                    (*readen) -= 2;
                }else{
                    (*readen)--;
                }
                break;
            case '\t':
                input[*readen] = '\0';
                (*readen)--;
                char *cmd_completed = tab_complete(input);
                if (cmd_completed != NULL) {
                    size_t len = strlen(cmd_completed);
                    input = Realloc(input, len + 1);
                    strncpy(input, cmd_completed, len);
                    *readen = len - 1;
                }
                break;
            case '\033':
                //esc key
                fread(input + *readen, sizeof(char), 1, stdin); //reading '['
                fread(input + *readen, sizeof(char), 1, stdin); //reading arrow type
                switch(input[*readen]) { // the real value
                    //todo
                    case 'A':
                        fprintf(stderr, "Up arrow key!\n");
                        break;
                    case 'B':
                        fprintf(stderr, "Down arrow key!\n");
                        break;
                    case 'C':
                        fprintf(stderr, "Right arrow key!\n");
                        break;
                    case 'D':
                        fprintf(stderr, "Left arrow key!\n");
                        break;
                    default:
                        fprintf(stderr, "A non-arrow key has been pressed\n");
                }
                (*readen)--;
                break;
            default:
                putchar(input[*readen]);
        }

        (*readen)++;
    }while(!terminate);

    input[*readen - 1] = '\0';

    return input;
}

/**
 * Tries to complete the string from the input with an appropriate output
 * @param input The incomplete input
 * @return ??? TODO
 */
char* tab_complete(char *input) {
    char* complete = search_command(input);
    if(complete == NULL)
        return NULL;
    for(int i = 0; i < strlen(input); i++){
        putchar(8);
        putchar(' ');
        putchar(8);
    }
    printf("%s", complete);
    return complete;
}

void *Realloc(void *buf, size_t size) {
    if(size < 0) {
        fprintf(stderr, "Error: tried to realloc a negative value\n");
    }else if((size) == 0){
        free(buf);
        buf = NULL;
    }else {

        int i = 0;
        do {
            void *oldbuf = buf;
            buf = realloc(buf, size);
            if (buf == NULL) {
                free(oldbuf);
            }
            i++;
        } while (buf == NULL && i < MAX_ATTEMPTS);
        if (buf == NULL) {
            perror("Realloc error");
            exit(1);
        }
    }
    return buf;
}

void *Malloc(size_t size) {
    if(size < 0){
        fprintf(stderr, "Error: tried to malloc a negative value\n");
        return 0;
    }else if(size == 0)
        fprintf(stderr, "Warning: malloc of zero size\n");

    void* pVoid = NULL;
    int i = 0;
    do {
        pVoid = malloc(size);
        i++;
    } while (pVoid == NULL && i < MAX_ATTEMPTS);
    if(pVoid == NULL){
        perror("Malloc error");
        exit(1);
    }
    return pVoid;
}

void reset_input_mode(){
    tcsetattr(STDIN_FILENO, TCSANOW, &saved_attributes);
}

void set_input_mode(){
    struct termios tattr;
    /* Make sure stdin is a terminal. */
    if (!isatty (STDIN_FILENO))
    {
        fprintf (stderr, "Stdin is not a terminal.\n");
        exit (EXIT_FAILURE);
    }

    /* Save the terminal attributes so we can restore them later. */
    tcgetattr (STDIN_FILENO, &saved_attributes);
    atexit (reset_input_mode);

    /* Set the funny terminal modes. */
    tcgetattr (STDIN_FILENO, &tattr);
    tattr.c_lflag &= ~(ICANON|ECHO); /* Clear ICANON and ECHO. */
    tattr.c_cc[VMIN] = 1;
    tattr.c_cc[VTIME] = 0;
    tcsetattr (STDIN_FILENO, TCSAFLUSH, &tattr);
}

/**
 * Tries to execute commands internal to the shell.
 * @param cmd
 * @param args
 */
int exec_local_cmd(char* cmd, char** args){
    if(strcmp(cmd, "cd") == 0){
        if(chdir(args[1]) == -1)
            perror("chdir error: ");
        return 1;
    }
    //todo: add bg for background commands
    return 0;
}
