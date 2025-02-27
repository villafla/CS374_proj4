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
// Uses material from Input Handling

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

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

// Function to parse user input
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
bool handle_builtin_commands(struct command_line *cmd) {
    if (cmd->argc == 0) return false;
    
    if (strcmp(cmd->argv[0], "exit") == 0) {
        exit(0);
    }
    
    if (strcmp(cmd->argv[0], "cd") == 0) {
        const char *target_dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
        if (chdir(target_dir) != 0) {
            perror("cd");
        }
        return true;
    }
    
    if (strcmp(cmd->argv[0], "status") == 0) {
        printf("exit value %d\n", last_exit_status);
        return true;
    }
    
    return false;
}

int main() {
    struct command_line *curr_command;
    
    while (true) {
        curr_command = parse_input();
        
        if (!curr_command) {
            continue;  // Ignore blank/comment lines
        }
        
        // Print parsed command (for debugging)
        printf("Command: ");
        for (int i = 0; i < curr_command->argc; i++) {
            printf("%s ", curr_command->argv[i]);
        }
        if (curr_command->input_file) {
            printf("< %s ", curr_command->input_file);
        }
        if (curr_command->output_file) {
            printf("> %s ", curr_command->output_file);
        }
        if (curr_command->is_bg) {
            printf("&");
        }
        printf("\n");
        
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

