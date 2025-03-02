/*
* Program Name: Programming Assignment 4: SMALLSH
* Author: Allyson Villaflor
* Email: villafla@oregonstate.edu
* CS 374 - Operating Systems I
* Program description: This program creates a shell called smallsh. smallsh implements a subset
*                      if well-known shells, such as bash. The program does the following:
*          
*                      - Provides a prompt for running commands
*                      - Handles blank lines and comments, which are lines beginning with the # character
*                      - Executes 3 commands exit, cd, and status via code built into the shell
*                      - Executes other commands by creating new processes using a function from 
*                        the exec() family of functions
*                      - Supports input and output redirection
*                      - Supports running commands in foregrounf and background processes
*                      - Implements custom handlers for 2 signals, SIGINT SIGTSTP
*/

/*
* Citation: The following function adapts code from sample_parser.c
* 
*/

#include "parser.h"

struct command_line *parse_input() {
    char input[INPUT_LENGTH];
    struct command_line *curr_command = calloc(1, sizeof(struct command_line));

    // Display shell prompt
    printf(": ");
    fflush(stdout);

    // Get input from user
    if (!fgets(input, INPUT_LENGTH, stdin)) {
        printf("\n");
        exit(0);
    }

    // Ignore blank lines and comments
    if (input[0] == '#' || input[0] == '\n') {
        free(curr_command);
        return NULL;
    }

    // Tokenize the input into arguments
    char *token = strtok(input, " \n");
    while (token) {
        if (!strcmp(token, "<")) {
            curr_command->input_file = strdup(strtok(NULL, " \n"));
        } else if (!strcmp(token, ">")) {
            curr_command->output_file = strdup(strtok(NULL, " \n"));
        } else if (!strcmp(token, "&")) {
            curr_command->is_bg = true;
        } else {
            curr_command->argv[curr_command->argc++] = strdup(token);
        }
        token = strtok(NULL, " \n");
    }
    curr_command->argv[curr_command->argc] = NULL;  // Null-terminate the argument list
    return curr_command;
}
