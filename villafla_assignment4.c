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

// The following code implements the sample parser 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define INPUT_LENGTH 2048
#define MAX_ARGS 512

// Struct to store parsed command
struct command_line {
    char *argv[MAX_ARGS + 1];  // Arguments (NULL-terminated)
    int argc;                  // Argument count
    char *input_file;          // Input file (if any)
    char *output_file;         // Output file (if any)
    bool is_bg;                // Background process flag
};

// Global variable to track last exit status
int last_exit_status = 0;

// // Function Prototypes
// bool handle_builtin_commands(struct command_line *cmd);
// void execute_command(struct command_line *cmd);

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
        // printf("Exiting shell...\n");
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
        // Print last foreground process exit status
        if (WIFEXITED(last_exit_status)) {
            printf("exit value %d\n", WEXITSTATUS(last_exit_status));
        } else if (WIFSIGNALED(last_exit_status)) {
            printf("terminated by signal %d\n", WTERMSIG(last_exit_status));
        }
        return true;
    }

    return false;
}

// Function to execute non-built-in commands
// Citation: Modules Process API, Exec API
void execute_command(struct command_line *cmd) {
    pid_t spawn_pid = fork();

    if (spawn_pid == -1) {
        perror("fork failed");
        last_exit_status = 1;
        return;
    }

    if (spawn_pid == 0) {
        // Child process
        execvp(cmd->argv[0], cmd->argv);

        // If execvp() fails, print error and exit
        perror("command not found");
        exit(1);
    } 
    else {
        // Parent process
        if (cmd->is_bg) {
            // Background process: Print PID and do NOT wait
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
        } 
        else {
            // Foreground process: Wait for it to finish
            int child_status;
            waitpid(spawn_pid, &child_status, 0);

            if (WIFEXITED(child_status)) {
                last_exit_status = WEXITSTATUS(child_status);
            } 
            else if (WIFSIGNALED(child_status)) {
                last_exit_status = WTERMSIG(child_status);
                printf("terminated by signal %d\n", last_exit_status);
            }
        }
    }
}

int main() {
    struct command_line *curr_command;
    pid_t bg_pids[100]; // Array to store background process PIDs
    int bg_count = 0;    // Number of background processes
    
    while (true) {
        // Periodically check background processes before showing prompt
        for (int i = 0; i < bg_count; i++) {
            int child_status;
            pid_t result = waitpid(bg_pids[i], &child_status, WNOHANG);
            if (result > 0) { // Background process has finished
                printf("background pid %d is done: ", bg_pids[i]);
                if (WIFEXITED(child_status)) {
                    printf("exit value %d\n", WEXITSTATUS(child_status));
                } else if (WIFSIGNALED(child_status)) {
                    printf("terminated by signal %d\n", WTERMSIG(child_status));
                }
                fflush(stdout);
                // Remove PID from array by shifting left
                for (int j = i; j < bg_count - 1; j++) {
                    bg_pids[j] = bg_pids[j + 1];
                }
                bg_count--; // Reduce background count
                i--; // Adjust index after removal
            }
        }

        curr_command = parse_input();
        
        if (!curr_command) {
            continue;  // Ignore blank/comment lines
        }
        
        if (!handle_builtin_commands(curr_command)) {
            pid_t spawn_pid = fork();

            if (spawn_pid == -1) {
                perror("fork failed");
                exit(1);
            } else if (spawn_pid == 0) {
                // Child process executes the command
                execvp(curr_command->argv[0], curr_command->argv);
                perror("execvp failed"); // Only runs if execvp fails
                exit(1);
            } else {
                // Parent process
                if (curr_command->is_bg) {
                    // Background process: Print PID and store it
                    printf("background pid is %d\n", spawn_pid);
                    fflush(stdout);
                    bg_pids[bg_count++] = spawn_pid;
                } else {
                    // Foreground process: Wait for completion
                    int child_status;
                    waitpid(spawn_pid, &child_status, 0);
                    if (WIFEXITED(child_status)) {
                        last_exit_status = WEXITSTATUS(child_status);
                    } else if (WIFSIGNALED(child_status)) {
                        last_exit_status = WTERMSIG(child_status);
                        printf("terminated by signal %d\n", last_exit_status);
                    }
                }
            }
        }
        
        // Free allocated memory
        for (int i = 0; i < curr_command->argc; i++) {
            free(curr_command->argv[i]);
        }
        free(curr_command->input_file);
        free(curr_command->output_file);
        free(curr_command);
    }

    return EXIT_SUCCESS;
}



