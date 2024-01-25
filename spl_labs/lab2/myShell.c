#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
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
            int inputFile = open(pCmdLine->inputRedirect, O_RDONLY);
            if (inputFile == -1) {
                perror("open inputRedirect");
                _exit(EXIT_FAILURE);
            }
            dup2(inputFile, STDIN_FILENO);
        }

        // Redirect output if outputRedirect is not NULL
        if (pCmdLine->outputRedirect != NULL) {
            int outputFile = open(pCmdLine->outputRedirect, O_WRONLY | O_CREAT | O_TRUNC, 0666);
            if (outputFile == -1) {
                perror("open outputRedirect");
                _exit(EXIT_FAILURE);
            }
            dup2(outputFile, STDOUT_FILENO);
        }

        printf("Executing command: %s\n", pCmdLine->arguments[0]);

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

            printf("Command Line: ");
            for (int i = 0; parsedCmd->arguments[i] != NULL; i++) {
                printf("%s ", parsedCmd->arguments[i]);
            }
            printf("\n");

            execute(parsedCmd);
            freeCmdLines(parsedCmd);
        }
    }

    return 0;
}



