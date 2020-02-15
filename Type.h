//
// Created by 陈瀚泽 on 10/8/19.
//

#ifndef OSH_TYPE_H
#define OSH_TYPE_H

#include <stdint.h>
#include <zconf.h>

typedef enum {
    no_redirect, create_file, add_to_file, read_from_stdin
}redirect_options_t;

typedef enum {
    front_operation, change_directory, exit_shell, clean_screen
}command_type_t;

typedef struct command{
    char * exec;
    char * file_name;
    char * * argument;
    int background_task;

    command_type_t command_type;
    struct command * next_command;
    redirect_options_t redirectOptions;
} command_t;

#endif //OSH_TY// PE_H

