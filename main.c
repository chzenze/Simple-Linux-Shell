//
// Created by 陈瀚泽 on 2019-06-07.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#define MAXLINE 80


int main(void)
{
    char *args[MAXLINE/2 + 1]; /* command line arguments */
    int shouldrun = 1; /* flag to determine when to exit program */
    while (shouldrun) {
        char *arg[128];
        int heastory = 0;
        bzero(arg, sizeof(char*)*128);
        char command[sysconf(_SC_ARG_MAX)];

        printf("osh>");
        fflush(stdout);

        reset:
        //read commend as [command] and [argument]
        fgets (command, sysconf(_SC_ARG_MAX), stdin);

        args[heastory] = malloc(sizeof(&command));
        strcpy(args[heastory],command);

        if(heastory++ >= MAXLINE/2)
            heastory = 0;

        //split the command
        int headindex =0;
        char * tmp_char = strtok(strtok(command,"\n")," ");
        while (tmp_char){
            arg[headindex] = malloc(strlen(tmp_char) * sizeof(char));
            strcpy(arg[headindex],tmp_char);
            tmp_char = strtok (NULL," ");
            headindex++;
        }

        if(strcmp(arg[0], "cd") == 0){
            if (chdir(arg[1]) == -1)
                printf("folder %s  is not exsist", arg[1]);
        }else if (arg[0][0] == '!'){
            if(arg[0][1] == '!')
                write(stdin, args[heastory], sizeof(&args[heastory]));
            else
                write(STDIN_FILENO, args[arg[0][0]], sizeof(&args[arg[0][0]]));
            write(STDIN_FILENO, "\n", 1);
            goto reset;
        }else if(strcmp(arg[0], "exit") == 0)
            shouldrun = 0;
        else{
            pid_t child = fork();
            if(child == 0){
                if (execvp(arg[0], arg) == -1)
                    printf("command %s not found \n", arg[0]);
            }else
                wait(NULL);
        }
    /**
        After reading user input, the steps are:
            (1) fork a child process using fork()
            (2) the child process will invoke execvp()
            (3) if command included &, parent will invoke wait()
    */
    }
    return 0;
}