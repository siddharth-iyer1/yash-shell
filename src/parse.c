#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 100

char** get_command() {
    char *input = readline("yash# ");
    char **args = malloc(MAX_ARGS * sizeof(char*));

    if (input == NULL) {
        printf("\n");
        return NULL;
    }

    int i = 0;
    char *token = strtok(input, " ");

    while (token != NULL && i < MAX_ARGS - 1) {
        args[i] = strdup(token);  // copy token into args
        if (args[i] == NULL) {
            perror("Unable to allocate memory for token");
            free(input);
            for (int j = 0; j < i; j++) {
                free(args[j]);
            }
            free(args);
            return NULL;
        }
        i++;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;

    free(input);
    return args;
}
