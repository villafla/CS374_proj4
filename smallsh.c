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
* Citation: The following function adapts code Exploration: Signal
* Handling API and Exploration: Process API - Creating and Terminating Processes. 
*/

/*
* Function: main
* ----------------------------------
* The entry point of the program. Initializes signal handlers,
* handles user input, and manages background processes.
* This function continuously prompts the user for commands,
* processes input, executes built-in or external commands,
* and handles foreground/background execution.
* 
* Arguments: None.
* 
* Returns: int - EXIT_SUCCESS (0) if the program runs successfully.
*/

#include "smallsh.h"
#include "parser.h"
#include "commands.h"
#include "signals.h"

// Global variables
int last_exit_status = 0;       // Tracks last exit status
int foreground_only_mode = 0;   // Tracks foreground only mode, 1 = enabled, 0 = disabled

int main() {
    struct command_line *curr_command;
    pid_t bg_pids[MAX_ARGS];  // Store background PIDs
    int bg_count = 0;         // Number of background processes

    // Set up SIGINT handler (Ctrl+C should NOT terminate the shell)
    struct sigaction sa_sigint = {0};
    sa_sigint.sa_handler = signal_SIGINT; // Ignore SIGINT
    sigfillset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = SA_RESTART; // Restart interrupted system calls
    sigaction(SIGINT, &sa_sigint, NULL);

    // Set up SIGTSTP handler (Ctrl+Z toggles foreground-only mode)
    struct sigaction sa_sigtstp = {0};
    sa_sigtstp.sa_handler = signal_SIGTSTP;
    sigfillset(&sa_sigtstp.sa_mask);
    sa_sigtstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_sigtstp, NULL);

    while (true) {
        // Check for finished background processes BEFORE showing prompt
        for (int i = 0; i < bg_count; i++) {
            int child_status;
            pid_t result = waitpid(bg_pids[i], &child_status, WNOHANG);
            if (result > 0) { // Background process finished
                if (WIFEXITED(child_status)) {
                    printf("background pid %d is done: exit value %d\n", result, WEXITSTATUS(child_status));
                } else if (WIFSIGNALED(child_status)) {
                    printf("background pid %d is done: terminated by signal %d\n", result, WTERMSIG(child_status));
                }
                fflush(stdout);
                // Remove from list
                bg_pids[i] = bg_pids[--bg_count];  
            }
        }
        
        // Get and process user input
        curr_command = parse_input();
        if (!curr_command) continue;  // Ignore blank/comment lines

        // Check if command is a built-in command, otherwise execute external
        if (!builtin_commands(curr_command, bg_pids, &bg_count)) {
            execute_other_commands(curr_command, bg_pids, &bg_count);
        }
        

        // Free memory allocate memory for command
        for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);
    }
    return EXIT_SUCCESS;
}

