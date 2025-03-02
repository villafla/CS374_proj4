#ifndef SMALLSH_H
#define SMALLSH_H

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

// Struct to store parsed command
struct command_line {
    char *argv[MAX_ARGS + 1];
    int argc;
    char *input_file;
    char *output_file;
    bool is_bg;
};

// Global variables
extern int last_exit_status;
extern int foreground_only_mode;

// Function prototypes
struct command_line *parse_input();
bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);
void execute_other_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);
void signal_SIGINT(int signo);
void signal_SIGTSTP(int signo);

#endif
