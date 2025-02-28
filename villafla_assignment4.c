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

// 150/170 on Gradescope 
// The following code implements the sample parser 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>  
#include <signal.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

// Function declaration 
// void handle_SIGINT(int signo);

// Struct to store parsed command
struct command_line {
    char *argv[MAX_ARGS + 1];  // Arguments (NULL-terminated)
    int argc;                  // Argument count
    char *input_file;          // Input file (if any)
    char *output_file;         // Output file (if any)
    bool is_bg;                // Background process flag
};

// Global variables 
int last_exit_status = 0;       // Tracks last exit status
int foreground_only_mode = 0;   // Tracks foreground-only mode, 1 = enabled, 0 = disabled

// Function to parse user input
// Citation: Module Input Handling
struct command_line *parse_input() {
    char input[INPUT_LENGTH];
    struct command_line *curr_command = calloc(1, sizeof(struct command_line));
    
    // Display shell prompt
    printf(": ");
    fflush(stdout);
    
    // Get input from user
    if (!fgets(input, INPUT_LENGTH, stdin)) {
        // Handle Ctrl+D (EOF)
        printf("\n");
        exit(0);
    }
    
    // Ignore blank lines and comments
    if (input[0] == '#' || input[0] == '\n') {
        free(curr_command);
        return NULL;
    }
    
    // Tokenize input
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
    curr_command->argv[curr_command->argc] = NULL;  // Null-terminate argv
    return curr_command;
}

// Function to handle built-in commands
// Citation: Module Environment Variables
bool handle_builtin_commands(struct command_line *cmd) {
    if (cmd->argc == 0) return false;  // Ignore empty commands

    if (strcmp(cmd->argv[0], "exit") == 0) {
        // Terminate any background processes before exiting
        fflush(stdout);
        exit(0);
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        // Ignore '&' in built-in commands
        if (cmd->is_bg) {
            cmd->is_bg = false;
        }

        const char *target_dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");

        if (chdir(target_dir) != 0) {
            perror("cd");
        }

        return true;
    }

    if (strcmp(cmd->argv[0], "status") == 0) {
        if (WIFEXITED(last_exit_status)) {
            printf("exit value %d\n", WEXITSTATUS(last_exit_status)); 
        } else if (WIFSIGNALED(last_exit_status)) {
            printf("terminated by signal %d\n", WTERMSIG(last_exit_status)); 
        }
        fflush(stdout);
        return true;
    }
       

    return false;
}

// Function to execute non-built-in commands with I/O redirection
// Citation: Modules Process API, Exec API
void execute_command(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
    // If foreground-only mode is enabled, force command to run in foreground
    if (foreground_only_mode) {
        cmd->is_bg = false;
    }
    
    pid_t spawn_pid = fork();

    if (spawn_pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (spawn_pid == 0) {
        // Child process

        // Restore SIGINT default behavior for foreground processes
        struct sigaction sa_child = {0};
        sa_child.sa_handler = SIG_DFL;
        sigaction(SIGINT, &sa_child, NULL);

        // Handle input redirection
        if (cmd->input_file) {
            int input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1) {
                fprintf(stderr, "%s: cannot open %s for input\n", cmd->argv[0], cmd->input_file);
                exit(1);
            }
            dup2(input_fd, 0); // Redirect stdin
            close(input_fd);
        } 
        // Redirect stdin to /dev/null for background processes if no input file is given
        else if (cmd->is_bg) {
            int dev_null = open("/dev/null", O_RDONLY);
            dup2(dev_null, 0);
            close(dev_null);
        }

        // Handle output redirection
        if (cmd->output_file) {
            int output_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                fprintf(stderr, "%s: cannot open %s for output\n", cmd->argv[0], cmd->output_file);
                exit(1);
            }
            dup2(output_fd, 1); // Redirect stdout
            close(output_fd);
        } 
        // Redirect stdout to /dev/null for background processes if no output file is given
        else if (cmd->is_bg) {
            int dev_null = open("/dev/null", O_WRONLY);
            dup2(dev_null, 1);
            close(dev_null);
        }

        // Execute the command
        execvp(cmd->argv[0], cmd->argv);
        
        // If execvp fails, print error message correctly
        perror(cmd->argv[0]);  // Prints "<command>: error message"
        exit(1);
    } 
    else {
        // Parent process
        if (cmd->is_bg) {
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            bg_pids[(*bg_count)++] = spawn_pid;  // Store background PID
        } else {
            // Foreground process: wait for child to finish
            int child_status;
            waitpid(spawn_pid, &child_status, 0);

            if (WIFEXITED(child_status)) {
                last_exit_status = child_status; // store full status
            } else if (WIFSIGNALED(child_status)) {
                last_exit_status = child_status; // store full status
                printf("terminated by signal %d\n", WTERMSIG(child_status));
                fflush(stdout);
            }
            
        }
    }
}


// Function Signal handler for SIGINT (Ctrl+C) - prevents shell termination
// Citation: Signal Handling API 
void handle_SIGINT(int signo) {
    write(STDOUT_FILENO, "\n: ", 3); // Show prompt again
    fflush(stdout);
}

// Function Signal hanlder for SIGTSTP (Ctrl+Z) - toggles foreground-only mode
// Citation: Signal Handling API 
void handle_SIGTSTP(int signo) {
    char* message;
    
    if (foreground_only_mode) {
        foreground_only_mode = 0;
        message = "\nExiting foreground-only mode\n: ";
    } else {
        foreground_only_mode = 1;
        message = "\nEntering foreground-only mode (& is now ignored)\n: ";
    }
    
    write(STDOUT_FILENO, message, strlen(message)); // Use write() instead of printf()
    fflush(stdout);
}

// Citation: Signal Handling API
int main() {
    struct command_line *curr_command;
    pid_t bg_pids[MAX_ARGS];  // Store background PIDs
    int bg_count = 0;         // Number of background processes

    // Set up SIGINT handler (Ctrl+C should NOT terminate the shell)
    struct sigaction sa_sigint = {0};
    sa_sigint.sa_handler = handle_SIGINT; // Ignore SIGINT
    sigfillset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = SA_RESTART; // Restart interrupted syscalls
    sigaction(SIGINT, &sa_sigint, NULL);

    // Set up SIGTSTP handler (Ctrl+Z toggles foreground-only mode)
    struct sigaction sa_sigtstp = {0};
    sa_sigtstp.sa_handler = handle_SIGTSTP;
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

        curr_command = parse_input();
        if (!curr_command) continue;  // Ignore blank/comment lines
        
        if (!handle_builtin_commands(curr_command)) {
            execute_command(curr_command, bg_pids, &bg_count);
        }

        // Free memory
        for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);
    }
    return EXIT_SUCCESS;
}
