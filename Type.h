//
// Created by 陈瀚泽 on 10/8/19.
//

#ifndef OSH_TYPE_H
#define OSH_TYPE_H

typedef enum {
    background_operation, front_operation, change_directory, exit_shell, clean_screen, type_nothing, env_list, redirect_output
}operation_t;

typedef enum{
    pipe_to_file, pipe_from_stdin, pipe_to_stdout, no_operation
}pipe_operation;

typedef struct command{
    int size;
    char connector;
    char ** command_array;
} command_t;
#endif //OSH_TY// PE_H

