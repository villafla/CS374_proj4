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
* Citation: The following functions adapts code from Exploration: 
* Environment, Exploration: Process API - Creating and Termination Processes, 
* Exploration: Process API - Executing a New Program, Exploration: Process
* API - Monitoring Child Processes, and Exploration: Processes and I/O.
* 
*/

#include "commands.h"

/*
* Function: builtin_commands
* ----------------------------------
* Handles built-in commands like exit, cd, and status.
* 
* Arguments: cmd - The parsed command structure
*            bg_pids - An array of background process IDs
*            bg_count - The number of background processes
* 
* Returns: True if a built-in command was handled, otherwise returns False.
*/

bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
    if (cmd->argc == 0) return false;

    // Map command strings to integer values
    int command_type = -1; 
    if (strcmp(cmd->argv[0], "exit") == 0) command_type = 0;
    else if (strcmp(cmd->argv[0], "cd") == 0) command_type = 1;
    else if (strcmp(cmd->argv[0], "status") == 0) command_type = 2;

    switch (command_type) {
        case 0: // "exit"
            for (int i = 0; i < *bg_count; i++) {
                kill(bg_pids[i], SIGTERM);  // Terminate all bg processes
            }
            exit(0);
            break;

        case 1: // "cd"
            if (cmd->is_bg) cmd->is_bg = false;
            const char *target_dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
            if (chdir(target_dir) != 0) perror("cd");
            return true;

        case 2: // "status"
            if (WIFEXITED(last_exit_status)) {
                printf("exit value %d\n", WEXITSTATUS(last_exit_status)); 
            } else if (WIFSIGNALED(last_exit_status)) {
                printf("terminated by signal %d\n", WTERMSIG(last_exit_status)); 
            }
            fflush(stdout);
            return true;

        default:
            return false;   // Not a built-in command
    }

    return true;    // Command was handled
}

// bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
//     if (cmd->argc == 0) return false;   // Ignore empty commands

//     // Handle "exit" command
//     if (strcmp(cmd->argv[0], "exit") == 0) {
//         // Kill all background processes before exiting
//         for (int i = 0; i < *bg_count; i++) { 
//             kill(bg_pids[i], SIGTERM);
//         }
//         exit(0);
//     }

//     // Handle "cd" command
//     if (strcmp(cmd->argv[0], "cd") == 0) {
//         // Prevent cd from running in the bg
//         if (cmd->is_bg) cmd->is_bg = false;
//         // If an argument is provided, use it as the target directory
//         const char *target_dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
//         // Change directory and handle errors
//         if (chdir(target_dir) != 0) perror("cd");
//         return true;
//     }

//     // Handle "status" command
//     if (strcmp(cmd->argv[0], "status") == 0) {
//         // Print the exit status or termination signal of the last fg process
//         if (WIFEXITED(last_exit_status)) {
//             printf("exit value %d\n", WEXITSTATUS(last_exit_status)); 
//         } else if (WIFSIGNALED(last_exit_status)) {
//             printf("terminated by signal %d\n", WTERMSIG(last_exit_status)); 
//         }
//         fflush(stdout);
//         return true;
//     }

//     return false;
// }

/*
* Function: execute_other_commands
* ----------------------------------
* Executes external commands using fork() and execvp().
* Handles input/output redirection and background execution.
* 
* Arguments: cmd - The parsed command structure
*            bg_pids - An array of background process IDs
*            bg_count - The number of background processes
* 
* Returns: void
*/

void execute_other_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
    // If fg-only mode is active, force the process to run in the fg
    if (foreground_only_mode) cmd->is_bg = false;

    pid_t spawn_pid = fork();
    if (spawn_pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (spawn_pid == 0) {    // Child process
        struct sigaction sa_ignore = {0};
        sa_ignore.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &sa_ignore, NULL);   // Ignore SIGTSTP in child process

        // Handle input redirection
        if (cmd->input_file) {
            int input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1) {
                fprintf(stderr, "cannot open %s for input\n", cmd->input_file);
                exit(1);
            }
            dup2(input_fd, 0);
            close(input_fd);
        }

        // Handle output redirection
        if (cmd->output_file) {
            int output_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                fprintf(stderr, "cannot open %s for output\n", cmd->output_file);
                exit(1);
            }
            dup2(output_fd, 1); // Redirect stdout
            close(output_fd);
        }

        // Execute the command
        execvp(cmd->argv[0], cmd->argv);

        // If execvp fails, print an error message and exit
        perror(cmd->argv[0]);
        exit(1);
    } else {    // Parent process
        if (cmd->is_bg) {
            // If bg process, store the PID and print message
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            bg_pids[(*bg_count)++] = spawn_pid;
        } else {
            // If foreground process, wait for it to finish
            int child_status;
            waitpid(spawn_pid, &child_status, 0);
            last_exit_status = child_status;    // Store exit status
        }
    }
}
