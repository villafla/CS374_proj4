#include "smallsh.h"
#include "parser.h"
#include "commands.h"
#include "signals.h"

int last_exit_status = 0;
int foreground_only_mode = 0;

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
        
        if (!handle_builtin_commands(curr_command, bg_pids, &bg_count)) {
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

