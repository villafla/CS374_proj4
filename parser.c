#include "parser.h"

// The following code implements the sample parser 
// Citation: Module Input Handling
struct command_line *parse_input() {
    char input[INPUT_LENGTH];
    struct command_line *curr_command = calloc(1, sizeof(struct command_line));

    printf(": ");
    fflush(stdout);

    if (!fgets(input, INPUT_LENGTH, stdin)) {
        printf("\n");
        exit(0);
    }

    if (input[0] == '#' || input[0] == '\n') {
        free(curr_command);
        return NULL;
    }

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
    curr_command->argv[curr_command->argc] = NULL;
    return curr_command;
}
