#include "signals.h"

// Citation: Signal Handling API
void signal_SIGINT(int signo) {
    write(STDOUT_FILENO, "\n: ", 3);
    fflush(stdout);
}

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
