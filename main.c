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

// check which command
command_type_t get_command_type(char * args) {
    if (strcmp(args, "cd") == 0)
        return change_directory;
    if (strcmp(args, "exit") == 0)
        return exit_shell;
    if (strcmp(args,"pause") == 0)
        return pause_shell;
    if (strcmp(args,"echo") == 0)
        return echo_command;
    if (strcmp(args, "clr") == 0)
        return clean_screen;
    return front_operation;
}

int to_next_space(const char * command){
    int head_count = 0;
    while (command[head_count] != ' ') head_count ++;
    return head_count;
}

command_t * parsing_command(char * command){
    size_t length = sizeof(command_t);
    command_t * head_command = calloc(1,length);
    command_t * current_command = head_command;

    for (unsigned long i = 0; i < strlen(command); i++) {
        //if meet end of string return
        if (command[i] == ' ') break;

        //assign new spaces for struct
        current_command->exec = strndup(command + i, to_next_space(command));

        //set command type
        current_command->command_type = get_command_type(current_command->exec);

        //fill in argument
        for(; command[i] != '|'; i++){

        }

        //move to next node of linklist;
        current_command->next_command = calloc(1,length);
        current_command =  current_command->next_command;
    }
}


//use for ececute the command
int exec_command(command_t * command) {
    int fd[2];
    int child_pid = 0;
    int redirect_file = 0;

    while(command->next_command){
        switch (command->command_type) {
            case exit_shell:
                return 1;
            case clean_screen:
                printf("\033[H\033[J");
                break;
            case echo_command:
                printf("%s\n", command->argument[0]);
                break;
            case pause_shell:
                printf("please press [ENTER] to continue\n");
                while (getchar() == '\n');
                break;
            case change_directory:
                if (chdir(command->argument[0]) == -1)
                    printf("Can't open folder %s\n", command->argument[0]);
                break;
            case redirect:
                switch (command->redirectOptions){
                    case create_file:
                        redirect_file = open(command->file_name, O_CREAT | O_TRUNC| O_WRONLY,0666);
                        break;
                    case add_to_file:
                        redirect_file = open(command->file_name, O_APPEND | O_WRONLY,0666);
                        break;
                    case read_from_stdin:
                        break;
                }
            case front_operation:
                if(command->command_type == ){
                    pipe(fd);
                    redirect_file = fd[1];
                }
                if ((child_pid = fork()) == 0) {
                    if(redirect_file != 0)
                        dup2(redirect_file,STDOUT_FILENO);
                    if(fd[0] != 0 && commands[i]->connector != '|')
                        dup2(fd[0],STDIN_FILENO);
                    if (execvp(current->exec, current->argument) == -1)
                        printf("command %s not found \n", current->exec);
                    exit(1);
                } else {
                    if (background_run_flag == 0) {
                        waitpid(child_pid, NULL, WNOHANG);
                        if(redirect_file != 0) {
                            dup2(STDOUT_FILENO, redirect_file);
                            redirect_file = 0;
                        }
                        if(fd[0] != 0 && commands[i]->connector != '|')
                            dup2(STDIN_FILENO,fd[0]);
                    } else
                        wait(200); //wait a small amount time for let execvp run
                }
        }
    }
}

int main(int argc, char *argv[]) {
    int max_index = 0;
    int max_command_size = sysconf(_SC_ARG_MAX);
    char history[MAX_HISTORY][max_command_size];

    memset(history,'\0', MAX_HISTORY * max_command_size);

    do{
        printf("osh>"),fflush(stdin);
        max_index = (max_index + 1) % MAX_HISTORY;
        memset(history[max_index],0, max_command_size);
        fgets(history[max_index], max_command_size, stdin);
        if(history[max_index][0] == '!')
            do{
                printf("\033[H\033[J");
                printf("osh>"),fflush(stdin);
                printf("%s", history[max_index - 1]);
            }while (getchar() == '\n')
    }while (exec_command(parsing_command(history[max_index])));
}
