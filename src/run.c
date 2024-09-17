#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>
#include <fcntl.h>

#define MAX_JOBS 20

typedef enum { RUNNING, STOPPED, DONE } job_status;

typedef struct {
    int job_id;
    pid_t pid;
    char *command;
    job_status status;
    int is_foreground;
} job;

job job_list[MAX_JOBS];
int job_count = 0;

void add_job(pid_t pid, char *command, job_status status, int is_foreground) {
    if (job_count < MAX_JOBS) {
        job_list[job_count].job_id = job_count;
        job_list[job_count].pid = pid;
        job_list[job_count].command = strdup(command);
        job_list[job_count].status = status;
        job_list[job_count].is_foreground = is_foreground;
        job_count++;
    } else {
        printf("Max job limit reached.\n");
    }
}

void remove_job(pid_t pid) {
    for (int i = 0; i < job_count; i++) {
        if (job_list[i].pid == pid) {
            free(job_list[i].command);
            for (int j = i; j < job_count - 1; j++) {
                job_list[j] = job_list[j + 1];
            }
            job_count--;
            break;
        }
    }
}

void print_job(int job_index, char sign) {
    printf("[%d] %c %s       %s\n", job_list[job_index].job_id, sign,
        job_list[job_index].status == RUNNING ? "Running" :
        job_list[job_index].status == STOPPED ? "Stopped" : "Done",
        job_list[job_index].command
        );
}

void check_background_jobs() {
    int status;
    pid_t pid;

    for (int i = 0; i < job_count; i++) {
        if (job_list[i].status == RUNNING) {
            pid = waitpid(job_list[i].pid, &status, WNOHANG);
            if (pid == job_list[i].pid) {
                if (WIFEXITED(status) || WIFSIGNALED(status)) {
                    job_list[i].status = DONE;
                    // printf("Job [%d] (%s) finished!\n", job_list[i].job_id, job_list[i].command);
                    print_job(i, '-');
                    remove_job(pid);
                }
            }
        }
    }
}

void print_jobs() {
    check_background_jobs();
    for (int i = 0; i < job_count; i++) {
        char sign = '-';
        if (i == job_count - 1 && job_list[i].is_foreground == 1){
            sign = '+';
        }
        print_job(i, sign);
    }
}

void fg_command() {
    for (int i = job_count - 1; i >= 0; i--) {
        if (job_list[i].status == STOPPED || (job_list[i].status == RUNNING && !job_list[i].is_foreground)) {
            job_list[i].status = RUNNING;
            job_list[i].is_foreground = 1;
            printf("%s", job_list[i].command);

            tcsetpgrp(STDIN_FILENO, job_list[i].pid);
            kill(-job_list[i].pid, SIGCONT);
            int status;
            waitpid(-job_list[i].pid, &status, WUNTRACED);

            // restore to shell after wait
            tcsetpgrp(STDIN_FILENO, getpid());

            if (WIFSTOPPED(status)) {
                job_list[i].status = STOPPED;
            } else {
                remove_job(job_list[i].pid);
            }
            break;
        }
    }
}


void bg_command() {
    for (int i = job_count - 1; i >= 0; i--) {
        if (job_list[i].status == STOPPED) {
            job_list[i].status = RUNNING;
            job_list[i].is_foreground = 0;
            print_job(i, '-');
            kill(job_list[i].pid, SIGCONT);
            break;
        }
    }
}

char* concatenate_args(char **args) {
    if (args == NULL) return NULL;

    int total_length = 0;
    int i = 0;

    while (args[i] != NULL) {
        total_length += strlen(args[i]) + 1;
        i++;
    }

    char *command_string = malloc(total_length);
    if (command_string == NULL) {
        perror("malloc failed");
        exit(1);
    }
    command_string[0] = '\0';

    for (i = 0; args[i] != NULL; i++) {
        strcat(command_string, args[i]);
        if (args[i + 1] != NULL) {
            strcat(command_string, " ");
        }
    }

    return command_string;
}

char* concatenate_pipeline_commands(char *left_command, char *right_command) {
    int total_length = strlen(left_command) + strlen(right_command) + 4;
    char *pipeline_command = malloc(total_length);
    if (pipeline_command == NULL) {
        perror("malloc failed");
        exit(1);
    }
    return pipeline_command;
}

void execute_command(char **args) {
    // parse the tokenized command for full command string
    char *command_string = concatenate_args(args);

    int i = 0;
    int in_fd = -1, out_fd = -1, err_fd = -1;
    int background = 0;

    // background
    while (args[i] != NULL) {
        if (strcmp(args[i], "&") == 0) {
            background = 1;
            args[i] = NULL;
            break;
        }
        i++;
    }

    // must happen in parent for blocking
    if (strcmp(args[0], "fg") == 0) {
        fg_command();
        return;
    } else if (strcmp(args[0], "bg") == 0) {
        bg_command();
        return;
    } else if (strcmp(args[0], "jobs") == 0) {
        print_jobs();
        return;
    }

    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            in_fd = open(args[i + 1], O_RDONLY);
            if (in_fd < 0) {
                perror("Failed to open input file");
                return;
            }
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            out_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (out_fd < 0) {
                perror("Failed to open output file");
                return;
            }
            args[i] = NULL;
        } else if (strcmp(args[i], "2>") == 0) {
            err_fd = open(args[i + 1], O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (err_fd < 0) {
                perror("Failed to open error file");
                return;
            }
            args[i] = NULL;
        }
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;

    } else if (pid == 0) {  // child process
        setpgid(0, 0);

        if (!background) {
            tcsetpgrp(STDIN_FILENO, getpid());
        }

        signal(SIGINT, SIG_DFL);
        signal(SIGTSTP, SIG_DFL);

        if (in_fd >= 0) {
            dup2(in_fd, STDIN_FILENO);
            close(in_fd);
        }
        if (out_fd >= 0) {
            dup2(out_fd, STDOUT_FILENO);
            close(out_fd);
        }
        if (err_fd >= 0) {
            dup2(err_fd, STDERR_FILENO);
            close(err_fd);
        }

        if (execvp(args[0], args) < 0) {
            perror("Execution failed");
            exit(1);
        }
        exit(0);

    } else {  // parent process
        setpgid(pid, pid);

        if (!background) {
            // foreground
            tcsetpgrp(STDIN_FILENO, pid);

            int status;
            waitpid(pid, &status, WUNTRACED);

            // restore terminal control to shell
            tcsetpgrp(STDIN_FILENO, getpid());

            if (WIFSTOPPED(status)) {
                // ^z
                add_job(pid, command_string, STOPPED, 1);
            } else {
                remove_job(pid);
            }
        } else {

            add_job(pid, command_string, RUNNING, 0);
        }

        check_background_jobs();
    }
}