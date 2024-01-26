#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include "LineParser.h"


void execute(cmdLine *pCmdLine);
void wakeup(int pid);
void nuke(int pid);

int debug = 0;

// Function to wake up a process
void wakeup(int pid) {
    // Check if the process is still alive
    if (kill(pid, 0) == -1) {
        perror("kill");
        fprintf(stderr, "Process %d is not alive\n", pid);
    } else {
        // Wake up the process
        if (kill(pid, SIGCONT) == -1) {
            perror("kill");
            fprintf(stderr, "Failed to wake up process %d\n", pid);
        } else {
            printf("Woke up process %d\n", pid);
        }
    }
}

// Function to terminate a process
void nuke(int pid) {
    if (kill(pid, SIGKILL) == -1) {
        perror("kill");
        fprintf(stderr, "Failed to terminate process %d\n", pid);
    } else {
        printf("Terminated process %d\n", pid);

        // Add a small delay before checking the process status
        sleep(1);

        // Wait for the process to terminate and collect its exit status
        int status;
        pid_t result;
        if ((result = waitpid(pid, &status, 0)) > 0) {
            if (WIFEXITED(status)) {
                printf("Process %d terminated with status %d\n", pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Process %d terminated by signal %d\n", pid, WTERMSIG(status));
            } else {
                printf("Process %d terminated\n", pid);
            }
        } else if (result == 0) {
            printf("Process %d is now a zombie\n", pid);
        } else {
            perror("waitpid");
            fprintf(stderr, "Error waiting for process %d to terminate\n", pid);
        }
    }
}


void execute(cmdLine *pCmdLine) {
    pid_t pid = fork();

    if (pid == -1) {
        perror("fork");
        if (debug == 1) {
            fprintf(stderr, "PID: %d\n", pid);
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        exit(EXIT_FAILURE);
    } else if (pid == 0) {

        // Redirect input if needed
        if (pCmdLine->inputRedirect != NULL) {
            int inputFd;
            if ((inputFd = open(pCmdLine->inputRedirect, O_RDONLY)) == -1){
                printf("Input file open error\n");
                _exit(1);
            }
            else{
                dup2(inputFd, 0);
                close(inputFd);
            }
        }

        // Redirect output if needed
        if (pCmdLine->outputRedirect != NULL) {
            int outputFd;
            if ((outputFd = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666)) == -1) {
                printf("Output file open error\n");
                _exit(1);
            } else {
                dup2(outputFd, 1);  // Redirect standard output to the outputFile
                close(outputFd);
            }
        }

        if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
            perror("execv problem");
            if (debug == 1) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
            }
            exit(EXIT_FAILURE);
        }
    } else {
        if (debug == 1) {
            fprintf(stderr, "PID: %d\n", pid);
            fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
        }
        if (pCmdLine->blocking) {
            // Parent process (shell)
            int status;
            if (waitpid(pid, &status, 0) == -1) {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        }
    }
}


int main(int argc, char **argv){
    char input[2048];
    cmdLine *parsedCmd;
    for (int i=1; i< argc; i++){
        if (strcmp("-d", argv[i]) == 0){
            debug = 1;
        }
    }
    while (1) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s$ ", cwd);
        } else {
            perror("getcwd");
            exit(EXIT_FAILURE);
        }
        if (fgets(input, 2048, stdin) == NULL) {
            perror("fgets");
            exit(EXIT_FAILURE);
        }

        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[len - 1] = '\0';
        }

        if (strcmp(input, "quit") == 0) {
            printf("Exiting the shell.\n");
            break;
        }

        if (strncmp(input, "cd", 2) == 0){
            char* path = input + 3;
            if (chdir(path) == -1){
                fprintf(stderr, "cd: %s: No such file or directory\n", path);
            }
        }
        else if (strncmp(input, "wakeup", 6) == 0) {
            // Parse the process id to wake up
            int pid = atoi(input + 7);
            wakeup(pid);
        } else if (strncmp(input, "nuke", 4) == 0) {
            // Parse the process id to terminate
            int pid = atoi(input + 5);
            nuke(pid);
        }

        else{
            parsedCmd = parseCmdLines(input);

            if (parsedCmd->inputRedirect != NULL) {
                printf("Input redirection from: %s\n", parsedCmd->inputRedirect);
            }
            if (parsedCmd->outputRedirect != NULL) {
                printf("Output redirection to: %s\n", parsedCmd->outputRedirect);
            }

            execute(parsedCmd);
            freeCmdLines(parsedCmd);
        }
    }

    return 0;
}


