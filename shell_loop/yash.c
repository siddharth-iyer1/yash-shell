#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_ARGS 100

int main() {
    char *input;
    char *args[MAX_ARGS];  // Array to store arguments
    int i;

    while (1) {
        input = readline("yash# ");

        // ctrl-D
        if (input == NULL) {
            printf("\n");
            return NULL;
        }

        if (strlen(input) > 0) {
            add_history(input);

        // remove any leading spaces
        char *trimmed_input = input;
        while (*trimmed_input == ' ') trimmed_input++;

            i = 0;
            args[i] = strtok(trimmed_input, " ");
            while (args[i] != NULL) {
                i++;
                args[i] = strtok(NULL, " ");
            }

            printf("Parsed arguments:\n");
            for (int j = 0; j < i; j++) {
                printf("args[%d]: %s\n", j, args[j]);
            }

            if (strcmp(args[0], "exit") == 0) {
                free(input);
                break;
            }

        }

        free(input);
    }

    return 0;
}
