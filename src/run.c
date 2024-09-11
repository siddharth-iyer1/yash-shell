#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <signal.h>

#define MAX_JOBS 20

typedef enum { RUNNING, STOPPED, DONE } job_status;

typedef struct {
    int job_id;
    pid_t pid;
    char *command;
    job_status status;
} job;

job job_list[MAX_JOBS];
int job_count = 0;
int next_job_id = 1;

void add_job(pid_t pid, char *command, job_status status) {
    if (job_count < MAX_JOBS) {
        job_list[job_count].job_id = next_job_id++;
        job_list[job_count].pid = pid;
        job_list[job_count].command = strdup(command);
        job_list[job_count].status = status;
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
            printf("Bringing job [%d] to background: %s\n", job_list[i].job_id, job_list[i].command);
            kill(job_list[i].pid, SIGCONT);
            break;
        }
    }
}

void execute_command(char **args) {
    int i = 0;
    while (args[i] != NULL) {
        i++;
    }

    int background = 0;

    // Check if the last argument is "&"
    if (i > 0 && strcmp(args[i-1], "&") == 0) {
        background = 1;
        args[i-1] = NULL;  // Remove the "&" from the args array
    }

    pid_t pid = fork();

    if (pid < 0) {
        perror("Fork failed");
        return;

    } else if (pid == 0) {  // Child process
        if (strcmp(args[0], "fg") == 0) {
            fg_command();
        } else if (strcmp(args[0], "bg") == 0) {
            bg_command();
        } else if (strcmp(args[0], "jobs") == 0) {
            printf("jobs\n");
            printf("jobs\n");
            printf("jobs\n");
            printf("jobs\n");
            printf("jobs\n");
        } else {
            // Execute the command
            if (execvp(args[0], args) < 0) {
                perror("Execution failed");
                exit(1);
            }
        }
    } else {  // Parent process
        if (background) {
            printf("Running command in the background, PID: %d\n", pid);
            add_job(pid, args[0], RUNNING);
        } else {
            // Wait for the child process to finish
            int status;
            waitpid(pid, &status, 0);
        }
    }
}