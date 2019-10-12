//
// Created by 陈瀚泽 on 10/8/19.
//

#ifndef OSH_TYPE_H
#define OSH_TYPE_H

typedef enum {
    background_operation, front_operation, change_directory, exit_shell, clean_screen, type_nothing, env_list, redirect_output, help_command
    ,Read_file, pause_shell, echo_command
}operation_t;

typedef enum{
    pipe_to_file, pipe_from_stdin, pipe_to_stdout, no_operation
}pipe_operation;

typedef struct command{
    int size;
    char connector;
    char *command_array[128];
} command_t;

char * help = "cd <directory> : Change the current default directory to <directory>. If the <directory> argument is not present, report the current directory. If the directory does not exist, an appropriate error message should be displayed. This command should also change the PWD environment variable.\nclr : Clear the Screen\ndir <directory> : List the contents of directory <directory>\nenviron : List all the environment strings\necho<comment> : Display <comment> on the display, followed by a new line (multiple spaces/tabs may be reduced to a single space)\nhelp : Display the user manual using more filter\npause : Pause the operation of the shell until “ENTER/RETURN” key is pressed\nquit : Quit the shell";
#endif //OSH_TY// PE_H

