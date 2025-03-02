#ifndef COMMANDS_H
#define COMMANDS_H

#include "smallsh.h"

bool builtin_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);
void execute_other_commands(struct command_line *cmd, pid_t *bg_pids, int *bg_count);

#endif
