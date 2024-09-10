#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

char* get_command() {
    char *input = readline("yash# ");
    add_history(input);
    return input;
}

