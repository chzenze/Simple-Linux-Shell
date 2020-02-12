//
// Created by 陈瀚泽 on 10/8/19.
//

#ifndef OSH_TYPE_H
#define OSH_TYPE_H

#include <stdint.h>

typedef enum {
    create_file, add_to_file, read_from_stdin
}redirect_options_t;

typedef enum {
    front_operation, change_directory, exit_shell, clean_screen, redirect
    , pause_shell, echo_command
}command_type_t;

typedef struct command{
    char * exec;
    char * argument[];
    char * file_name;
    uint32_t argument_size;
    command_type_t command_type;
    struct command * next_command;
    redirect_options_t redirectOptions;
} command_t;

#endif //OSH_TY// PE_H

