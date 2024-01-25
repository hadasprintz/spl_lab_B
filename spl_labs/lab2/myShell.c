#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include "LineParser.h"


void execute(cmdLine *pCmdLine);

int debug = 0;

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

        // Redirect input if inputRedirect is not NULL
        if (pCmdLine->inputRedirect != NULL) {
            FILE *inputFile = fopen(pCmdLine->inputRedirect, "r");
            if (inputFile == NULL) {
                perror("open inputRedirect");
                _exit(EXIT_FAILURE);
            }
            dup2(fileno(inputFile), STDIN_FILENO);
        }

        if (pCmdLine->outputRedirect != NULL) {
            FILE *outputFile = fopen(pCmdLine->outputRedirect, "w");
            if (outputFile == NULL) {
                perror("open outputRedirect");
                _exit(EXIT_FAILURE);
            }
            dup2(fileno(outputFile), STDOUT_FILENO);
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

        else{
            parsedCmd = parseCmdLines(input);
            execute(parsedCmd);
            freeCmdLines(parsedCmd);
        }
    }

    return 0;
}



