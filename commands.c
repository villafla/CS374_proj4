#include "commands.h"

// Citation: Module Environment Variables
bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
    if (cmd->argc == 0) return false;

    if (strcmp(cmd->argv[0], "exit") == 0) {
        for (int i = 0; i < *bg_count; i++) { 
            kill(bg_pids[i], SIGTERM);
        }
        exit(0);
    }

    if (strcmp(cmd->argv[0], "cd") == 0) {
        if (cmd->is_bg) cmd->is_bg = false;
        const char *target_dir = (cmd->argc > 1) ? cmd->argv[1] : getenv("HOME");
        if (chdir(target_dir) != 0) perror("cd");
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

// Citation: Modules Process API, Exec API
void execute_other_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count) {
    if (foreground_only_mode) cmd->is_bg = false;

    pid_t spawn_pid = fork();
    if (spawn_pid == -1) {
        perror("fork failed");
        exit(1);
    } else if (spawn_pid == 0) {
        struct sigaction sa_ignore = {0};
        sa_ignore.sa_handler = SIG_IGN;
        sigaction(SIGTSTP, &sa_ignore, NULL);

        if (cmd->input_file) {
            int input_fd = open(cmd->input_file, O_RDONLY);
            if (input_fd == -1) {
                fprintf(stderr, "cannot open %s for input\n", cmd->input_file);
                exit(1);
            }
            dup2(input_fd, 0);
            close(input_fd);
        }

        if (cmd->output_file) {
            int output_fd = open(cmd->output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (output_fd == -1) {
                fprintf(stderr, "cannot open %s for output\n", cmd->output_file);
                exit(1);
            }
            dup2(output_fd, 1);
            close(output_fd);
        }

        execvp(cmd->argv[0], cmd->argv);
        perror(cmd->argv[0]);
        exit(1);
    } else {
        if (cmd->is_bg) {
            printf("background pid is %d\n", spawn_pid);
            fflush(stdout);
            bg_pids[(*bg_count)++] = spawn_pid;
        } else {
            int child_status;
            waitpid(spawn_pid, &child_status, 0);
            last_exit_status = child_status;
        }
    }
}
