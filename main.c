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

char shell_location[PATH_MAX];
// check which command
operation_t command_handler(char *args[], int argument_size) {
    if (args[0][0] == '\0')
        return type_nothing;
    if (strcmp(args[0], "environ") == 0)
        return env_list;
    if (strcmp(args[0], "cd") == 0)
        return change_directory;
    if (strcmp(args[0], "exit") == 0)
        return exit_shell;
    if (strcmp(args[0],"pause") == 0)
        return pause_shell;
    if (strcmp(args[0],"echo"))
        return echo_command;
    if (strcmp(args[0], "clr") == 0)
        return clean_screen;
    if(strcmp(args[0],"help") == 0)
        return help_command;
    if(strcmp(args[0],"myshell") == 0)
        return Read_file;
    if (strstr(args[argument_size - 1], "&"))
        return background_operation;
    for (int i = 0; i < argument_size; ++i) {
        if (strcmp(args[i], ">>") == 0 || strcmp(args[i], ">") == 0)
            return redirect_output;
    }
    return front_operation;
}

//clean the arguments
void clean_args(command_t *args[], int argument_size) {
    for (int i = 0; i < argument_size + 1; ++i) {
        free(args[i]);
        args[i] = NULL;
    }
}

//use for ececute the command
int exec_command(char *command, char **envp) {
    int fd[2];
    int head_index = 0;
    int command_size = 0;
    int redirect_file = 0;
    command_t *args[128];
    void *source = NULL;
    int background_run_flag = 0;

    memset(args, 0, sizeof(command_t *) * 128);
    for (unsigned long i = 0; i < strlen(command); i++) {
        start:
        if(!args[command_size]) {
            args[command_size] = malloc(sizeof(command_t));
            memset(args[command_size], 0, sizeof(command_t));
        }

        //if meet end of string return
        if (command[i] == '\n'){
            break;
        }

        //skip all spaces
        while (command[i] == ' ') {
            i++;
            head_index++;
        }

        //find the end index for next argument
        while (command[i + 1] != ' ' && command[i + 1] != '\n') {
            if (command[i + 1] == '|' || command[i + 1] == ';') {

                //write the end of command
                args[command_size]->command_array[args[command_size]->size] = strndup(command + head_index,i - head_index + 1);

                //add command counter
                args[command_size]->size = args[command_size]->size + 1;

                //write connector
                args[command_size]->connector = command[i + 1];

                //set start point
                i++;
                command_size++;
                head_index = i + 1;
                goto start;
            }
            i++;
        }
        args[command_size]->command_array[args[command_size]->size] = strndup(command + head_index, i - head_index + 1);

        head_index = i + 1;
        args[command_size]->size = args[command_size]->size + 1;
    }

    for (int i = 0; i < command_size + 1; i++){
        int argument_size = args[i]->size;
        char ** arg = args[i]->command_array;

        switch (command_handler(arg, argument_size)) {
            case exit_shell:
                return 1;
            case type_nothing:
                return 0;
            case clean_screen:
                printf("\033[H\033[J");
                break;
            case echo_command:
                printf("%s\n",arg[1]);
                return 0;
            case env_list:
                for (char **env = envp; *env != 0; env++) {
                    char *thisEnv = *env;
                    printf("%s\n", thisEnv);
                }
                break;
            case pause_shell:
                printf("please press [ENTER] to continue");
                while (getchar() == '\n');
                break;
            case change_directory:
                if (chdir(arg[1]) == -1)
                    printf("folder %s  is not exsist", arg[1]);
                break;
            case Read_file:
                source = fopen(args[0]->command_array[1], "r");

                if (source == NULL) {
                    printf("File [%s] is not exist", args[0]->command_array[1]);
                    return -1;
                }

                while (fgets(command, sysconf(_SC_ARG_MAX), source))
                    exec_command(command, envp);
                return 0;
            case background_operation:
                background_run_flag = 1;
                free(arg[argument_size - 1]);
                arg[argument_size - 1] = NULL;
                goto run;
            case help_command:
                clean_args(args,command_size);
                args[0] = malloc(sizeof(command_t));
                memset(args[0],0, sizeof(command_t));

                args[0]->command_array[0] = "more";
                args[0]->command_array[1] = malloc(sysconf(_SC_ARG_MAX)* sizeof(char));
                strcpy(args[0]->command_array[1],shell_location);
                strcat(args[0]->command_array[1],"/ReadMe.txt");
                goto run;
            case redirect_output:
                background_run_flag = 0;
                for (int j = 0; j < argument_size; ++j) {
                    if (strcmp(arg[j], ">>") == 0) {
                        redirect_file = open(arg[j + 1], O_CREAT | O_TRUNC| O_WRONLY,0666);
                        for (int z = j; z < argument_size; ++z) {
                            free(arg[z]);
                            arg[z] = NULL;
                        }
                        break;
                    } else if(strcmp(arg[j], ">") == 0){
                        redirect_file = open(arg[j + 1], O_APPEND | O_WRONLY,0666);
                        for (int z = j; z < argument_size; ++z) {
                            free(arg[z]);
                            arg[z] = NULL;
                        }
                        break;
                    }
                }
            run:
            case front_operation:
                if(args[i]->connector == '|'){
                    pipe(fd);
                    redirect_file = fd[1];
                }
                if (fork() == 0) {
                    if(redirect_file != 0)
                        dup2(redirect_file,STDOUT_FILENO);
                    if(fd[0] != 0 && args[i]->connector != '|')
                        dup2(fd[0],STDIN_FILENO);
                    if (execvp(arg[0], arg) == -1)
                        printf("command %s not found \n", arg[0]);
                    exit(1);
                } else {
                    if (background_run_flag == 0) {
                        wait(NULL);
                        if(redirect_file != 0) {
                            dup2(STDOUT_FILENO, redirect_file);
                            redirect_file = 0;
                        }
                        if(fd[0] != 0 && args[i]->connector != '|')
                            dup2(STDIN_FILENO,fd[0]);
                    } else
                        wait(200); //wait a small amount time for let execvp run
                }
        }
    }
    //clear the command cache
    clean_args(args, command_size);
    return 0;
}

int main(int argc, char *argv[], char **envp) {
    int running = 0;
    getcwd(shell_location,PATH_MAX * sizeof(char));

    while (running == 0) {
        char command[sysconf(_SC_ARG_MAX)];
        memset(command,0, sizeof(char) *sysconf(_SC_ARG_MAX));

        printf("myshell>");
        fflush(stdin);
        fgets(command, sysconf(_SC_ARG_MAX), stdin);

        if(command[0] != '\n')
            running = exec_command(command, envp);


        memset(command,0, sizeof(char) *sysconf(_SC_ARG_MAX));
    }
}
