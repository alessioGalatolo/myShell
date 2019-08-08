#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <termios.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "cmd_storage.h"

#define COMMAND_BASE_LENGTH 100
#define ARG_MAX_LENGTH 20
#define COMPLETE_FILE "./complete_file~"

#define COMMAND_LIST_COMMAND find /bin -executable -maxdepth 1

//max attempets for malloc, realloc, ...
#define MAX_ATTEMPTS 3

#define FREE(x)\
    if((x) != NULL){free(x); x = NULL;}

//keep attributes of terminal
struct termios saved_attributes;

void *Malloc(size_t size);

char *readcommand(size_t *readen);

void *Realloc(void *buf, size_t size);

void set_input_mode();

char* tab_complete(char *input);

int runcommand(char *args[]);

char *remove_new_line(char *s){
    char *ret = s;
    while(*(s + 1) != '\0')
        s++;
    *s = '\0';
    return ret;
}


//TODO change ARG_MAX_LENGTH
int main(){
    size_t input_length = COMMAND_BASE_LENGTH;
    char *input;
    char *command;



    set_input_mode();

    do{
        char *save_ptr;
        char *args[ARG_MAX_LENGTH];
        char* cwd = getcwd(NULL, 0);
//        write(1, cwd, strlen(cwd) * sizeof(char));
        printf("%s: ", cwd);
        FREE(cwd);

        input = readcommand(&input_length);
        args[0] = strtok_r(input, " ", &save_ptr);
        command = args[0];
        int i = 1;
        args[ARG_MAX_LENGTH - 1] = NULL;
        do{
            args[i] = strtok_r(NULL, " ", &save_ptr);
            i++;
        }while(i < ARG_MAX_LENGTH - 1 && args[i - 1] != NULL);
        args[i - 2] = remove_new_line(args[i - 2]);
        if(strncmp("exit", args[0], sizeof(char) * 4) != 0){
            store_command(args);
            runcommand(args);
        }
        FREE(input);
    }while(strncmp("exit", command, sizeof(char) * 4));

    return 0;
}

int runcommand(char *args[]) {
    char* command = args[0];
    int pid = fork();
    if(pid < 0){
        perror("fork error");
        return 1;
    }else if( pid == 0 ){
        //TODO: change length
        char cmd[COMMAND_BASE_LENGTH + 5];
        sprintf(cmd, "/bin/");
        strncat(cmd, command, COMMAND_BASE_LENGTH + 5);
        execvp(cmd, args);
        if(errno == ENOENT) {
            printf("\'%s\' command not found\n", command);
            sprintf(cmd, "/usr/bin/");
            strncat(cmd, command, COMMAND_BASE_LENGTH + 5);
            execvp(cmd, args);
        } else
            perror("Exec");
        return 1;
    }else {
        waitpid(pid, NULL, 0);
    }
    return 0;
}

char *readcommand(size_t *readen) {
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
            case 127:
                if (*readen > 0) {
                    input[*readen] = '\0';
                    putchar(8);
                    putchar(' ');
                    putchar(8);
                    (*readen) -= 2;
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
                    *readen = len;
                    input[*readen] = '\n';
                    terminate = 1;
                }
                break;
            default:
                putchar(input[*readen]);
        }

        (*readen)++;
    }while(!terminate);

    if(*readen >= input_length){
        input = Realloc(input, sizeof(char) * (*readen + 1));
        *readen += sizeof(char); //TODO: what?
    }

    input[*readen] = '\0';

    return input;
}

char* tab_complete(char *input) {
//    int mfd = open(COMPLETE_FILE, O_WRONLY, 0666);
//    int pid = fork();
//    if(pid < 0){
//        perror("fork error");
//        return 1;
//    }else if( pid == 0 ){
//        size_t len = strlen(input) + 1;
//        char to_complete[len];
//        strncpy(to_complete, input, len);
//        to_complete[len - 2] = '*';
//        to_complete[len - 1] = '\0';
//        //dup2(2, mfd);
//        dup2(1, mfd);
//        execl("/usr/bin/find", "/bin", "-name", to_complete, NULL);
//        printf("sei grullo\n");
//        return 1;
//    }else {
//        waitpid(pid, NULL, 0);
//        //elaborate output
//        //fprintf(stderr, "ecnsp\n");
//        close(mfd);
//        //unlink(COMPLETE_FILE);
//    }
    char* complete = search_command(input);
    if(complete == NULL)
        return NULL;
    for(int i = 0; i < strlen(input); i++){
        putchar(8);
        putchar(' ');
        putchar(8);
    }
    printf("%s\n", complete);
    return complete;
}


void *Realloc(void *buf, size_t size) {
    int i = 0;

    do {
        void* oldbuf = buf;
        buf = realloc(buf, size);
        if(buf == NULL) {
            free(oldbuf);
        }
        i++;
    } while (buf == NULL && i < MAX_ATTEMPTS);
    if(buf == NULL){
        perror("Realloc error");
        exit(1);
    }
    return buf;
}

void *Malloc(size_t size) {
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
    tcsetattr (STDIN_FILENO, TCSANOW, &saved_attributes);
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
