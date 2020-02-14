//
// Created by 陈瀚泽 on 10/8/19.
//
#include "Type.h"
#include <string.h>
#include <zconf.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define MAX_HISTORY 50

// check which command type
command_type_t get_command_type(char * args) {
    if (strcmp(args, "cd") == 0)
        return change_directory;
    if (strcmp(args, "exit") == 0)
        return exit_shell;
    if (strcmp(args,"echo") == 0)
        return echo_command;
    if (strcmp(args, "clr") == 0)
        return clean_screen;
    return front_operation;
}

//parsing char to command struct with linklist
command_t * parsing_command(char * command){
    char * arg_ptr, * str_ptr;
    size_t length = sizeof(command_t);
    command_t * head_command = calloc(1,length), * current_command = NULL;

    char * tmp_string = strtok_r(command,"|\n",&str_ptr);

    while(tmp_string){
        int arg_index = 0;
        char * tmp_argument;

        //move to next node of linklist;
        if(!current_command){
            current_command = head_command;
        }else{
            current_command->next_command = calloc(1,length);
            current_command =  current_command->next_command;
        }

        //find the exec name
        tmp_argument = strtok_r(tmp_string, " \n",&arg_ptr);
        current_command->exec = strdup(tmp_argument);
        while (tmp_argument){
            //handle redirection
            if(tmp_argument[0] == '>' || tmp_argument[0] == '<'){
                if(tmp_argument[1] == '>')
                    current_command->redirectOptions = create_file;
                else if (tmp_argument[0] == '<')
                    current_command->redirectOptions = read_from_stdin;
                else
                    current_command->redirectOptions = add_to_file;

                current_command->file_name = strdup(strtok_r(NULL, " \n",&arg_ptr));
                break;
            }
            //handle normal command
            if(!current_command->argument)
                current_command->argument = calloc(1, sizeof(char *));
            else
                current_command->argument = realloc(current_command->argument, (arg_index + 1) * sizeof(char *));

            current_command->argument[arg_index ++] = strdup(tmp_argument);
            tmp_argument = strtok_r(NULL, " \n", &arg_ptr);
        }

        //arg[] should be {filename, argument[], NULL}
        current_command->argument = realloc(current_command->argument, (arg_index + 1) * sizeof(char *));
        current_command->argument[arg_index] = NULL;

        tmp_string = strtok_r(NULL,"|",&str_ptr);
    }

    return  head_command;
}


//use for execute the command
int exec_command(command_t * command) {
    int fd[2];
    int child_pid = 0;
    char args;
    int write_file = 0, read_file = 0;

    while(command){
        switch (command->command_type) {
            case exit_shell:
                return 0;
            case clean_screen:
                printf("\033[H\033[J");
                break;
            case echo_command:
                printf("%s\n", command->argument[0]);
                break;
            case change_directory:
                if (chdir(command->argument[0]) == -1)
                    printf("Can't open folder %s\n", command->argument[0]);
                break;
            case front_operation:
                //handle output redirect
                pipe(fd);
                if (command->redirectOptions == add_to_file){
                    write_file = open(command->file_name, O_APPEND | O_WRONLY, 0666);
                } else if (command->redirectOptions == create_file){
                    write_file = open(command->file_name, O_CREAT | O_TRUNC | O_WRONLY, 0666);
                } else if (command->redirectOptions == read_from_stdin){
                    read_file = open(command->file_name, O_CREAT | O_WRONLY | O_TRUNC, 0666);
                }

                if ((child_pid = fork()) == 0) {
                    if(read_file != 0)
                        dup2(read_file,STDIN_FILENO);
                    if(write_file != 0)
                        dup2(write_file, STDOUT_FILENO);
                    if(fd[0] != 0 && !command->next_command)
                        dup2(fd[0],STDIN_FILENO);
                    if (execvp(command->exec, command->argument) == -1)
                        printf("command %s not found \n", command->exec);
                    exit(-1);
                } else {
                    if (command->background_task == 1) {
                        waitpid(child_pid, NULL, WNOHANG);
                        if(read_file != 0) {
                            dup2(STDIN_FILENO, read_file);
                            read_file = 0;
                        }
                        if(write_file != 0) {
                            dup2(STDOUT_FILENO, write_file);
                            write_file = 0;
                        }
                        if(fd[0] != 0 && !command->next_command)
                            dup2(STDIN_FILENO,fd[0]);
                    } else
                        wait(200); //wait a small amount time for let execvp run
                }
        }
        command = command->next_command;
    }

    return 1;
}

int main(int argc, char *argv[]) {
    int max_index = 0;
    char * history[MAX_HISTORY];
    int max_command_size = sysconf(_SC_ARG_MAX);
    char * line_buffer = calloc(1, max_command_size);

    memset(history,'\0', MAX_HISTORY);

    do{
        printf("osh>"),fflush(stdin);
        max_index = (max_index + 1) % MAX_HISTORY;
        fgets(line_buffer, max_command_size, stdin);
        history[max_index] = strndup(line_buffer,strlen(line_buffer));
        if(history[max_index][0] == '!')
            do{
                printf("\033[H\033[J");
                printf("osh>"),fflush(stdin);
                printf("%s", history[max_index - 1]);
            }while (getchar() == '\n');
    }while (exec_command(parsing_command(history[max_index])));
}
