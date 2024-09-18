#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/readline.h>
#include <readline/history.h>

#include "parse.h"
#include "run.h"

int main() {
    pid_t shell_pgid = getpid();
    if (setpgid(shell_pgid, shell_pgid) < 0) {
        exit(1);
    }

    tcsetpgrp(STDIN_FILENO, shell_pgid);

    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);

    while (1) {
        char ***commands = get_commands();
        if (!commands) {
            break;
        }
        if (commands[1] != NULL) {
            execute_piped_command(commands[0], commands[1]);
        } else {
            execute_command(commands[0]);
        }

        free(commands);
    }

    return 0;
}
