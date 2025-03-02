#include "smallsh.h"
#include "parser.h"
#include "commands.h"
#include "signals.h"

int last_exit_status = 0;
int foreground_only_mode = 0;

int main() {
    struct command_line *curr_command;
    pid_t bg_pids[MAX_ARGS];
    int bg_count = 0;

    struct sigaction sa_sigint = {0};
    sa_sigint.sa_handler = signal_SIGINT;
    sigfillset(&sa_sigint.sa_mask);
    sa_sigint.sa_flags = SA_RESTART;
    sigaction(SIGINT, &sa_sigint, NULL);

    struct sigaction sa_sigtstp = {0};
    sa_sigtstp.sa_handler = signal_SIGTSTP;
    sigfillset(&sa_sigtstp.sa_mask);
    sa_sigtstp.sa_flags = SA_RESTART;
    sigaction(SIGTSTP, &sa_sigtstp, NULL);

    while (true) {
        curr_command = parse_input();
        if (!curr_command) continue;

        if (!builtin_commands(curr_command, bg_pids, &bg_count)) {
            execute_other_commands(curr_command, bg_pids, &bg_count);
        }

        free(curr_command);
    }
    return EXIT_SUCCESS;
}
