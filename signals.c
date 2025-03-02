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
* Citation: The following functions adapts code Exploration: Signal
* Handling API.
*/

#include "signals.h"

/*
* Function: signal_SIGINT
* ----------------------------------
* Handles the SIGINT signal (Ctrl+C). Ensures that the shell
* does not terminate and redisplays the prompt.
* 
* Arguments: signo - The received signal number (SIGINT).
* 
* Returns: void
*/
void signal_SIGINT(int signo) {
    write(STDOUT_FILENO, "\n: ", 3);
    fflush(stdout);
}

/*
* Function: signal_SIGTSTP
* ----------------------------------
* Handles the SIGTSTP signal (Ctrl+Z). Toggles foreground-only mode
* and prints the appropriate message to notify the user.
* 
* Arguments: signo - The received signal number (SIGTSTP).
* 
* Returns: void
*/
// Citation: Signal Handling API
void signal_SIGTSTP(int signo) {
    char* message;
    if (foreground_only_mode) {
        foreground_only_mode = 0;
        message = "\nExiting foreground-only mode\n: ";
    } else {
        foreground_only_mode = 1;
        message = "\nEntering foreground-only mode (& is now ignored)\n: ";
    }
    write(STDOUT_FILENO, message, strlen(message));
    fflush(stdout);
}
