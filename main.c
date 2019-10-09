//
// Created by 陈瀚泽 on 10/8/19.
//
#include "Type.h"
#include <string.h>
#include <zconf.h>
#include <stdio.h>

operation_t command_handler(char * args[], int argument_size){
    if(strcmp(args[0],"cd") == 0)
        return change_directory;
    if (strcmp(args[0],"exit") == 0)
        return exit_shell;
    if(strstr(args[argument_size - 1],"&"))
        return background_operation;
    return front_operation;
}

void clean_args(char* args[], int argument_size){
    for (int i = 0; i < argument_size ; ++i) {
        if(args[i])
            free(args[i]);
    }
}

int command_parser(char * command, char *args[]){
    //split the command
    int head_index =0;
    int number_argument = 0;
    for(unsigned long i = 0; i < strlen(command); i++){
        if (command[i] == '\n')
            return number_argument;
        //remove all possible spaces
        while (command[i] == ' '){
            i ++;
            head_index ++;
        }

        //find the end index for next argument
        while (command[i + 1] != ' ' && command[i + 1] != '\n')
            i ++;

        //allocate the space for array
        args[number_argument] = malloc(sizeof(char) * (i - head_index + 1));

        //write argument to args
        strncpy(args[number_argument],command + head_index,i - head_index + 1);

        number_argument++;
        head_index = i + 1;
    }
    return number_argument;
}

int exec_command(char * args[], int argument_size){
    pid_t child = 0;
    int run_flag = 0;

    switch (command_handler(args,argument_size)) {
        case exit_shell:
            return 1;
            break;
        case change_directory:
            if (chdir(args[0]) == -1)
                printf("folder %s  is not exsist", args[1]);
            break;
        case background_operation:
            run_flag = 1;
            free(args[argument_size -1]);
            args[argument_size - 1] = NULL;
        case front_operation:
            child = fork();
            if (child == 0) {
                if (execvp(args[0],args) == -1)
                    printf("command %s not found \n", args[0]);
            } else {
                if (run_flag == 0)
                    wait(NULL);
                else
                    wait(200); //wait a small amount time for let execvp run
            }
            break;
    }
    return 0;
}

int main(int argc, char *argv[]){

    FILE *fp = NULL;
    int running = 0;
    char * args[128];
    void *source = NULL;
    memset(args,0, 128 * sizeof(char *));

    while (running == 0) {
        int argument_size = 0;
        char command[sysconf(_SC_ARG_MAX)];

        if(args < 2){
            printf("osh>");
            fflush(stdout);

            //read commend as [command] and [argument]
            source = stdin;
        }else{
            source = fopen(argv[1], "r");
            if(source == NULL){
                printf("File [%s] is not exist",argv[1]);
                return  -1;
            }
        }

        while(fgets(command, sysconf(_SC_ARG_MAX), source)){
            //parse command to char *[]
            argument_size = command_parser(command, &args);

            //running the command
            running = exec_command(args, argument_size);

            //clear the command cache
            clean_args(args, argument_size);
        }
    }
}
