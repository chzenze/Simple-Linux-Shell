//
// Created by 陈瀚泽 on 10/8/19.
//
#include "Type.h"
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/wait.h>

#define MAX_HISTORY 50

// check which command type
command_type_t get_command_type(char * args) {
    if (strcmp(args, "cd") == 0)
        return change_directory;
    if (strcmp(args, "exit") == 0)
        return exit_shell;
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
        current_command->background_task = strcmp(tmp_argument, "&");
        current_command->command_type = get_command_type(current_command->exec);
        while (tmp_argument){
            //handle redirection
            if(tmp_argument[0] == '>' || tmp_argument[0] == '<'){
                if(tmp_argument[0] == '>')
                    current_command->redirectOptions = create_file;
                else
                    current_command->redirectOptions = read_from_stdin;
                current_command->file_name = strdup(strtok_r(NULL, " \n",&arg_ptr));
                break;
            }
            //handle normal command
            if(!current_command->argument)
                current_command->argument = calloc(1, sizeof(char *));
            else
                current_command->argument = realloc(current_command->argument, (arg_index + 1) * sizeof(char *));

            current_command->argument[arg_index ++] = strdup(tmp_argument);
            tmp_argument = strtok_r(NULL, " &\n", &arg_ptr);
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
    int write_file = 0, read_file = 0;

    while(command){
        switch (command->command_type) {
            case exit_shell:
                return 0;
            case clean_screen:
                printf("\033[H\033[J");
                break;
            case change_directory:
                if (chdir(command->argument[0]) == -1)
                    printf("Can't open folder %s\n", command->argument[0]);
                break;
            case front_operation:
                //handle output redirect
                if(command->next_command)
                    pipe(fd);
                if (command->redirectOptions == add_to_file)
                    write_file = open(command->file_name, O_APPEND | O_WRONLY, 0666);
                else if (command->redirectOptions == create_file)
                    write_file = open(command->file_name, O_CREAT | O_TRUNC | O_WRONLY, 0666);
                else if (command->redirectOptions == read_from_stdin)
                    read_file = open(command->file_name, O_RDONLY, 0666);

                fflush(stdout);
                if (fork() == 0) {
                    if(read_file != 0)
                        dup2(read_file, STDIN_FILENO);
                    if(write_file != 0)
                        dup2(write_file, STDOUT_FILENO);
                    if(command->next_command)
                        dup2(fd[1],STDOUT_FILENO);
                    if(fd[0] != 0 && !command->next_command)
                        dup2(fd[0],STDIN_FILENO);
                    if (execvp(command->exec, command->argument) == -1)
                        printf("command %s not found \n", command->exec);
                    exit(1);
                } else {
                    if (command->background_task == 1) {
                        wait(NULL);
                        if(read_file != 0) {
                            dup2(STDIN_FILENO, read_file);
                            close(read_file);
                            read_file = 0;
                        }
                        if(write_file != 0) {
                            dup2(STDOUT_FILENO, write_file);
                            close(write_file);
                            write_file = 0;
                        }
                        if(command->next_command)
                            dup2(STDOUT_FILENO, fd[1]);
                        if(fd[0] != 0 && !command->next_command)
                            dup2(STDIN_FILENO,fd[0]);
                    }
                }
        }
        command = command->next_command;
    }
    return 1;
}

int main(int argc, char *argv[]) {
    int flag =1;
    int max_index = 0;
    char * history[MAX_HISTORY];
    int max_command_size = sysconf(_SC_ARG_MAX);
    char * line_buffer = calloc(1, max_command_size);

    memset(history,'\0', MAX_HISTORY);

    while(flag){
        start:
        fflush(stdout);
        printf("osh>"),fflush(stdin);
        max_index = (max_index + 1) % MAX_HISTORY;
        fgets(line_buffer, max_command_size, stdin);

        //command history
        if(line_buffer[0] == '!') {
            do {
                if(history[max_index]){
                    printf("\033[H\033[J");
                    printf("osh>");
                    printf("%s", history[max_index - 1]);
                    printf("press any key to see previous or press enter to exec");
                } else{
                    printf("No commands in history.\n");
                    goto start;
                }
            } while (getchar() == '\n');
        }

        //handle no command situation;
        if(line_buffer[0] != '\n')
            flag = exec_command(parsing_command(line_buffer));

        history[max_index] = strndup(line_buffer,strlen(line_buffer));

    }
}
