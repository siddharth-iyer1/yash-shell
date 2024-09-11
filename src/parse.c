#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 100

char** tokenize_command(char *input) {
    char **args = malloc(MAX_ARGS * sizeof(char*));

    int i = 0;
    char *token = strtok(input, " ");
    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = strdup(token);
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
    return args;
}

char*** get_commands() {
    char *input = readline("# ");
    if (input == NULL) {
        printf("\n");
        return NULL;
    }

    // Check if the command contains a pipe
    char *pipe_position = strchr(input, '|');
    char ***commands = malloc(2 * sizeof(char**));

    if (pipe_position != NULL) {
        *pipe_position = '\0';
        char *command1 = input;
        char *command2 = pipe_position + 1;

        commands[0] = tokenize_command(command1);
        commands[1] = tokenize_command(command2);
    } else {
        commands[0] = tokenize_command(input);
        commands[1] = NULL;
    }

    free(input);
    return commands;
}