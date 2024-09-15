#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parse.h"
#include "run.h"

int main() {
    while (1) {
        char ***commands = get_commands();
        if (commands == NULL) {
            break;
        }
        // Handle the commands

        if (commands[1] != NULL) {
            printf("Piped command detected!\n");
            printf("Command 1:\n");
            for (int i = 0; commands[0][i] != NULL; i++) {
                printf("  %s\n", commands[0][i]);
            }

            printf("Command 2:\n");
            for (int i = 0; commands[1][i] != NULL; i++) {
                printf("  %s\n", commands[1][i]);
            }

            // Run the piped commands (implement piping logic here)
        } else {
            printf("Single command:\n");
            for (int i = 0; commands[0][i] != NULL; i++) {
                printf("  %s\n", commands[0][i]);
            }
            execute_command(commands[0]);
        }

        free(commands);
    }

    return 0;
}
