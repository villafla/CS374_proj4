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

#ifndef COMMANDS_H
#define COMMANDS_H

#include "smallsh.h"

bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);
void execute_other_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);

#endif
