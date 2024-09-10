#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parse.h"

int main() {
    while (1) {
        char **command = get_command();  // Retrieve the array of tokens
        if (command == NULL) {
            continue;
        }
        int i = 0;
        while (command[i] != NULL) {
            printf("arg %d: %s\n", i, command[i]);
            i++;
        }
        free(command);
    }
    return 0;
}
