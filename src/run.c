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
int next_job_id = 1;

void add_job(pid_t pid, char *command, job_status status, int is_foreground) {
    if (job_count < MAX_JOBS) {
        job_list[job_count].job_id = next_job_id++;
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

void fg_command() {
    for (int i = job_count - 1; i >= 0; i--) {
        if (job_list[i].status == STOPPED || job_list[i].status == RUNNING) {
            job_list[i].status = RUNNING;
            job_list[i].is_foreground = 1;
            printf("Bringing job [%d] to foreground: %s\n", job_list[i].job_id, job_list[i].command);
            kill(job_list[i].pid, SIGCONT);

            int status;
            waitpid(job_list[i].pid, &status, 0);
            remove_job(job_list[i].pid);
            break;
        }
    }
}

void bg_command() {
    for (int i = job_count - 1; i >= 0; i--) {
        if (job_list[i].status == STOPPED) {
            job_list[i].status = RUNNING;
            job_list[i].is_foreground = 0;
            printf("Bringing job [%d] to background: %s\n", job_list[i].job_id, job_list[i].command);
            kill(job_list[i].pid, SIGCONT);
            break;
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

        printf("[%d] %c %s       %s\n", job_list[i].job_id, sign,
            job_list[i].status == RUNNING ? "Running" :
            job_list[i].status == STOPPED ? "Stopped" : "Done",
            job_list[i].command);
    }
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
                    printf("Job [%d] (%s) finished!\n", job_list[i].job_id, job_list[i].command);
                    remove_job(pid);
                }
            }
        }
    }
}

void execute_command(char **args) {
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
        if (background) {
            printf("Running command in the background, PID: %d\n", pid);
            add_job(pid, args[0], RUNNING, 0);
        } else {
            int status;
            waitpid(pid, &status, 0);   // foreground block
        }

        check_background_jobs();  // clears jobs
    }
}