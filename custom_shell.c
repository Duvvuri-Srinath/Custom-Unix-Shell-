#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_CMD_LEN 1024
#define MAX_ARGS 64
#define MAX_JOBS 64

typedef struct {
    pid_t pid;
    char command[MAX_CMD_LEN];
    int is_background;
} Job;

Job jobs[MAX_JOBS];
int job_count = 0;

// Signal handlers
void sigchld_handler(int sig) {
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        for (int i = 0; i < job_count; i++) {
            if (jobs[i].pid == pid) {
                printf("\n[Job %d] %s (PID: %d) completed.\n", i + 1, jobs[i].command, pid);
                jobs[i] = jobs[job_count - 1];
                job_count--;
                break;
            }
        }
    }
}

void sigint_handler(int sig) {
    printf("\nType 'exit' to quit the shell.\n");
}

void sigtstp_handler(int sig) {
    printf("\nForeground job stopped. Use 'fg' to resume.\n");
}

// Add job to the list
void add_job(pid_t pid, char *cmd, int is_background) {
    if (job_count < MAX_JOBS) {
        jobs[job_count].pid = pid;
        strncpy(jobs[job_count].command, cmd, MAX_CMD_LEN);
        jobs[job_count].is_background = is_background;
        job_count++;
    }
}

// Execute a command
void execute_command(char **args, int background) {
    pid_t pid = fork();
    if (pid == 0) {
        // Child process
        if (background) {
            printf("[Background Job] PID: %d\n", getpid());
        }
        execvp(args[0], args);
        perror("exec failed");
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        // Parent process
        add_job(pid, args[0], background);
        if (!background) {
            waitpid(pid, NULL, 0);
        }
    } else {
        perror("fork failed");
    }
}

// Parse input and execute
void parse_and_execute(char *input) {
    char *args[MAX_ARGS];
    int background = 0;
    
    // Tokenize input
    int i = 0;
    char *token = strtok(input, " \t\n");
    while (token != NULL) {
        if (strcmp(token, "&") == 0) {
            background = 1;
        } else {
            args[i++] = token;
        }
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;

    if (args[0] == NULL) return;

    // Built-in commands
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1]) {
            chdir(args[1]);
        } else {
            fprintf(stderr, "cd: missing argument\n");
        }
        return;
    }

    execute_command(args, background);
}

// Main shell loop
void shell_loop() {
    char input[MAX_CMD_LEN];
    
    // Set signal handlers
    signal(SIGCHLD, sigchld_handler);
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);

    while (1) {
        printf("custom-shell$ ");
        fflush(stdout);
        
        // Read user input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break; // Exit on EOF
        }

        parse_and_execute(input);
    }
}

int main() {
    printf("Custom Unix Shell Started. Type 'exit' to quit.\n");
    shell_loop();
    return 0;
}
