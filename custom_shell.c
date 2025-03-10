#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAX_INPUT_SIZE 1024
#define MAX_ARG_SIZE 100

void sigint_handler(int signo);
void sigtstp_handler(int signo);
void sigchld_handler(int signo);

void execute_command(char *input);
void parse_input(char *input, char **args);
int handle_builtin_commands(char **args);
void execute_piped_commands(char *input);
void execute_with_redirection(char **args);
void add_to_history(const char *input);

pid_t foreground_pid = -1;

void execute_command(char *input) {
    if (strchr(input, '|')) {
        execute_piped_commands(input);
        return;
    }

    char *args[MAX_ARG_SIZE];
    parse_input(input, args);
    
    if (args[0] == NULL) return;
    if (handle_builtin_commands(args)) return;

    execute_with_redirection(args);
}

void parse_input(char *input, char **args) {
    int i = 0;
    char *token = strtok(input, " ");
    while (token) {
        args[i++] = token;
        token = strtok(NULL, " ");
    }
    args[i] = NULL;
}

int handle_builtin_commands(char **args) {
    if (strcmp(args[0], "exit") == 0) {
        exit(0);
    } else if (strcmp(args[0], "cd") == 0) {
        if (args[1] == NULL) {
            fprintf(stderr, "cd: missing argument\n");
        } else {
            if (chdir(args[1]) != 0) {
                perror("cd");
            }
        }
        return 1;
    }
    return 0;
}

void execute_with_redirection(char **args) {
    int in_redirect = 0, out_redirect = 0;
    char *input_file = NULL, *output_file = NULL;
    
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "<") == 0) {
            in_redirect = 1;
            input_file = args[i + 1];
            args[i] = NULL;
        } else if (strcmp(args[i], ">") == 0) {
            out_redirect = 1;
            output_file = args[i + 1];
            args[i] = NULL;
        }
    }

    pid_t pid = fork();
    if (pid == 0) {
        if (in_redirect) {
            int fd = open(input_file, O_RDONLY);
            if (fd < 0) { perror("Error opening input file"); exit(1); }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        if (out_redirect) {
            int fd = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd < 0) { perror("Error opening output file"); exit(1); }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }

        execvp(args[0], args);
        perror("Execution failed");
        exit(1);
    } else if (pid > 0) {
        foreground_pid = pid;
        waitpid(pid, NULL, 0);
        foreground_pid = -1;
    } else {
        perror("Fork failed");
    }
}

void execute_piped_commands(char *input) {
    int pipe_fd[2], prev_pipe = -1;
    char *commands[MAX_ARG_SIZE];
    int num_commands = 0;
    
    char *token = strtok(input, "|");
    while (token) {
        commands[num_commands++] = token;
        token = strtok(NULL, "|");
    }

    for (int i = 0; i < num_commands; i++) {
        pipe(pipe_fd);
        pid_t pid = fork();

        if (pid == 0) {
            if (prev_pipe != -1) {
                dup2(prev_pipe, STDIN_FILENO);
                close(prev_pipe);
            }
            if (i < num_commands - 1) {
                dup2(pipe_fd[1], STDOUT_FILENO);
            }
            close(pipe_fd[0]);
            close(pipe_fd[1]);

            char *args[MAX_ARG_SIZE];
            parse_input(commands[i], args);
            execvp(args[0], args);
            perror("Execution failed");
            exit(1);
        } else if (pid > 0) {
            close(pipe_fd[1]);
            if (prev_pipe != -1) close(prev_pipe);
            prev_pipe = pipe_fd[0];
        } else {
            perror("Fork failed");
        }
    }

    while (wait(NULL) > 0);
}

void sigint_handler(int signo) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGINT);
    }
}

void sigtstp_handler(int signo) {
    if (foreground_pid > 0) {
        kill(foreground_pid, SIGTSTP);
    }
}

void sigchld_handler(int signo) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void add_to_history(const char *input) {
    add_history(input);
}

int main() {
    char *input;
    signal(SIGINT, sigint_handler);
    signal(SIGTSTP, sigtstp_handler);
    signal(SIGCHLD, sigchld_handler);

    while (1) {
        input = readline("\033[1;32mcustom-shell$ \033[0m");

        if (!input) break; // Handle EOF (Ctrl+D)
        if (strlen(input) == 0) continue; // Ignore empty input

        add_to_history(input);
        execute_command(input);
        free(input);
    }

    return 0;
}