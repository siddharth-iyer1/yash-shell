#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

void execute_command(char **args) {
    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;
    } else if (pid == 0) {      // child
        if (args[0] == "fg"){

        }
        else if (args[0] == "bg"){

        }
        else if (args[0] == "jobs"){

        }
        else if (execvp(args[0], args) < 0) {
            perror("Execution failed");
            exit(1);
        }
    } else {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
    }
}
