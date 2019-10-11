//
// Created by 陈瀚泽 on 10/8/19.
//
#include "Type.h"
#include <string.h>
#include <zconf.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/wait.h>

operation_t command_handler(char *args[], int argument_size) {
    if (args[0][0] == '\0')
        return type_nothing;
    if (strcmp(args[0], "environ") == 0)
        return env_list;
    if (strcmp(args[0], "cd") == 0)
        return change_directory;
    if (strcmp(args[0], "exit") == 0)
        return exit_shell;
    if (strcmp(args[0], "clr") == 0)
        return clean_screen;
    if (strstr(args[argument_size - 1], "&"))
        return background_operation;
    return front_operation;
}

void clean_args(char *args[], int argument_size) {
    for (int i = 0; i < argument_size; ++i) {
        if (args[i]) {
            memset(args[i], 0, strlen(args[i]) * sizeof(char));
            free(args[i]);
        }
    }
}

int command_parser(char *command, command_t *args[]) {

    int head_index = 0;
    int number_argument = 0;

    for (unsigned long i = 0; i < strlen(command); i++) {
        //if empty then create
        if (!args[number_argument]->command_array)
            args[number_argument]->command_array = malloc(sizeof(char *));
        else
            args[number_argument]->command_array = realloc(args[number_argument]->command_array, head_index * sizeof(char *));

        //if meet end of string return
        if (command[i] == '\n')
            return number_argument;

        //skip all spaces
        while (command[i] == ' ')
            i++;

        //find the end index for next argument
        while (command[i + 1] != ' ' && command[i + 1] != '\n') {
            if (command[i + 1] == '|' || command[i + 1] == ';') {
                number_argument++;

                //write the end of command
                number_argument++;
                arg[number_argument] = malloc(sizeof(char) * (i - head_index + 1));

                //write coomadn
                args[number_argument]->connector = command[i + 1];

                //set start point
                i++;
                head_index = i + 1;
                memset(args[number_argument]->command_array,'0',)
            }

        }

        //allocate the space for array
        args[number_argument]->command_array[args[number_argument]->size] = malloc(sizeof(char) * (i - head_index + 1));

        //write argument to args
        strncpy(args[number_argument]->command_array[args[number_argument]->size], command + head_index, i - head_index + 1);

        head_index = i + 1;
        args[number_argument]->size++;
    }
    return number_argument;
}

int exec_command(char *command, char **envp) {
    int argument_size = 0;
    pid_t child = 0;
    int pipe_file = 0;
    command_t **args = NULL;
    int background_run_flag = 0;


    //parse command to char *[]
    argument_size = command_parser(command, &args);

    switch (command_handler(args, argument_size)) {
        case exit_shell:
            return 1;
        case type_nothing:
            return 0;
        case clean_screen:
            printf("\033[H\033[J");
            break;
        case env_list:
            for (char **env = envp; *env != 0; env++) {
                char *thisEnv = *env;
                printf("%s\n", thisEnv);
            }
            break;
        case change_directory:
            if (chdir(args[0]) == -1)
                printf("folder %s  is not exsist", args[1]);
            break;
        case background_operation:
            background_run_flag = 1;
            free(args[argument_size - 1]);
            args[argument_size - 1] = NULL;
            goto run;
        case redirect_output:
            background_run_flag = 0;
            for (int i = 0; i < argument_size; ++i) {
                if (strcmp(args[i], ">>") == 0) {
                    pipe_file = open(args[i + 1], O_CREAT | O_WRONLY);
                    for (int j = i; j < argument_size; ++j) {
                        free(args[j]);
                        args[j] = NULL;
                    }
                    break;
                } else if(strcmp(args[i], ">>") == 0){
                    pipe_file = open(args[i + 1], O_APPEND | O_WRONLY);
                    for (int j = i; j < argument_size; ++j) {
                        free(args[j]);
                        args[j] = NULL;
                    }
                    break;
                }
            }
        run:
        case front_operation:
            child = fork();
            if (child == 0) {
                dup2(pipe_file,STDIN_FILENO);
                if (execvp(args[0], args) == -1)
                    printf("command %s not found \n", args[0]);
            } else {
                if (background_run_flag == 0) {
                    wait(NULL);
                } else {
                    wait(200); //wait a small amount time for let execvp run
                }
            }
            break;
    }

    //clear the command cache
    clean_args(args, argument_size);

    return 0;
}

int main(int argc, char *argv[], char **envp) {
    FILE *fp = NULL;
    int running = 0;
    void *source = NULL;

    while (running == 0) {
        char command[sysconf(_SC_ARG_MAX)];

        if (argc < 2) {
            printf("osh>");
            fflush(stdout);
            fgets(command, sysconf(_SC_ARG_MAX), stdin);

            running = exec_command(command, envp);
        } else {
            source = fopen(argv[1], "r");

            if (source == NULL) {
                printf("File [%s] is not exist", argv[1]);
                return -1;
            }

            while (fgets(command, sysconf(_SC_ARG_MAX), source))
                running = exec_command(command, envp);
        }
    }
}
