#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include "LineParser.h"

#define TERMINATED  -1
#define RUNNING 1
#define SUSPENDED 0

typedef struct process{
    cmdLine* cmd;                         /* the parsed command line*/
    pid_t pid; 		                  /* the process id that is running the command*/
    int status;                           /* status of the process: RUNNING/SUSPENDED/TERMINATED */
    struct process *next;	                  /* next process in chain */
} process;

#define HISTLEN 20
#define MAX_CMD_LENGTH 1024

typedef struct {
    char* command;
} HistoryEntry;

HistoryEntry* history[HISTLEN];
int newest = -1;
int oldest = -1;

void initHistory() {
    for (int i = 0; i < HISTLEN; i++) {
        history[i] = NULL;
    }
}

void addHistory(char* cmd) {
    if (newest == -1) {
        oldest = 0;
    }
    newest = (newest + 1) % HISTLEN;
    if (history[newest] != NULL) {
        free(history[newest]->command);
        free(history[newest]);
    }
    history[newest] = (HistoryEntry*)malloc(sizeof(HistoryEntry));
    history[newest]->command = strdup(cmd);
    if (oldest == newest) {
        oldest = (oldest + 1) % HISTLEN;
    }
}

void printHistory() {
    printf("History:\n");
    int index = oldest;
    int count = 1;
    while (index != -1 && index != newest) {
        printf("%d: %s\n", count, history[index]->command);
        index = (index + 1) % HISTLEN;
        count++;
    }
}

char* getLastCommand() {
    if (newest == -1) {
        return NULL;
    }
    return history[newest]->command;
}

char* getCommandByIndex(int index) {
    if (index < 1 || index > HISTLEN || index > newest - oldest + 1) {
        return NULL;
    }
    int histIndex = (oldest + index - 2) % HISTLEN;
    return history[histIndex]->command;
}

int debug = 0;

void addProcess(process** process_list, cmdLine* cmd, pid_t pid){
    process* new_process = malloc(sizeof(process));
    if (new_process == NULL){
        perror("problem with malloc");
        exit(EXIT_FAILURE);
    }
    new_process->cmd = cmd;
    new_process->status = RUNNING;
    new_process->pid = pid;
    new_process->next = *process_list;
    *process_list = new_process;
}

void printProcessList(process** process_list){
    printf("Index\tPID\tSTATUS\tCommand\n");
    process* this_process = *process_list;
    int index = 0;
    while (this_process != NULL){
        printf("%d\t%d\t", index, this_process->pid);
        if (this_process->status == TERMINATED){
            printf("Terminated\t");
        }
        else if (this_process->status == RUNNING){
            printf("Running\t\t");
        }
        else if (this_process->status == SUSPENDED){
            printf("Suspended\t");
        }
        printf("%s", this_process->cmd->arguments[0]);
        for (int i = 1; this_process->cmd->arguments[i] != NULL; i++){
            printf(" %s", this_process->cmd->arguments[i]);
        }
        printf("\n");
        this_process = this_process->next;
        index++;
    }
}

void freeProcessList(process* process_list) {
    process* current = process_list;
    while (current != NULL) {
        process* temp = current;
        current = current->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }
}

void updateProcessList(process** process_list) {
    process* current = *process_list;
    while (current != NULL) {
        int status;
        pid_t result = waitpid(current->pid, &status, WNOHANG | WUNTRACED | WCONTINUED);
        if (result == -1) {
            if (errno == ECHILD) {
                // No child processes
                break;
            } else {
                perror("waitpid");
                exit(EXIT_FAILURE);
            }
        } else if (result == 0) {
            // Process is still running
        } else {
            // Process has terminated or stopped
            if (WIFEXITED(status)) {
                current->status = TERMINATED;
            } else if (WIFSTOPPED(status)) {
                current->status = SUSPENDED;
            } else if (WIFCONTINUED(status)) {
                current->status = RUNNING;
            }
        }
        current = current->next;
    }
}

void updateProcessStatus(process* process_list, int pid, int status) {
    process* current = process_list;
    while (current != NULL) {
        if (current->pid == pid) {
            current->status = status;
            return;
        }
        current = current->next;
    }
    fprintf(stderr, "Process with PID %d not found in the process list\n", pid);
}

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
    if (kill(pid, SIGINT) == -1) {
        perror("kill");
        fprintf(stderr, "Failed to terminate process %d\n", pid);
    } else {
        printf("Terminated process %d\n", pid);
    }

    sleep(1);
}

void procs(process **process_list) {
    updateProcessList(process_list);
    printProcessList(process_list);
}

void suspendProcess(process **process_list, int pid) {
    if (kill(pid, SIGTSTP) == -1) {
        perror("kill");
        fprintf(stderr, "Failed to suspend process %d\n", pid);
    } else {
        printf("Suspended process %d\n", pid);
        updateProcessStatus(*process_list, pid, SUSPENDED);
    }
}

void nukeProcess(process **process_list, int pid) {
    if (kill(pid, SIGINT) == -1) {
        perror("kill");
        fprintf(stderr, "Failed to terminate process %d\n", pid);
    } else {
        printf("Terminated process %d\n", pid);
        updateProcessStatus(*process_list, pid, TERMINATED);
    }
}

void wakeupProcess(process **process_list, int pid) {
    if (kill(pid, SIGCONT) == -1) {
        perror("kill");
        fprintf(stderr, "Failed to wake up process %d\n", pid);
    } else {
        printf("Woke up process %d\n", pid);
        updateProcessStatus(*process_list, pid, RUNNING);
    }
}


void execute(cmdLine *pCmdLine, process** process_list) {
    if (pCmdLine->next) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("pipe problem");
            exit(EXIT_FAILURE);
        }

        pid_t pid_1 = fork();

        if (pid_1 == -1) {
            perror("fork");
            if (debug == 1) {
                fprintf(stderr, "PID: %d\n", pid_1);
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
            }
            exit(EXIT_FAILURE);
        } else if (pid_1 == 0) {
            close(pipe_fd[0]);
            dup(pipe_fd[1]);
            close(pipe_fd[1]);

            if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                perror("execv problem");
                if (debug == 1) {
                    fprintf(stderr, "PID: %d\n", pid_1);
                    fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
                }
                exit(EXIT_FAILURE);
            }

        } else {
            close(pipe_fd[1]);
            if (debug == 1) {
                fprintf(stderr, "PID: %d\n", pid_1);
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
            }
            pid_t pid_2 = fork();
            if (pid_2 == -1) {
                perror("fork");
                if (debug == 1) {
                    fprintf(stderr, "PID: %d\n", pid_2);
                    fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
                }
                exit(EXIT_FAILURE);
            } else if (pid_2 == 0) {
                close(pipe_fd[1]);
                dup(pipe_fd[0]);
                close(pipe_fd[0]);

                if (execvp(pCmdLine->arguments[0], pCmdLine->arguments) == -1) {
                    perror("execv problem");
                    if (debug == 1) {
                        fprintf(stderr, "PID: %d\n", pid_2);
                        fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
                    }
                    exit(EXIT_FAILURE);
                }
            } else {
                close(pipe_fd[0]);
                int status;
                waitpid(pid_1, &status, 0);
                waitpid(pid_2, &status, 0);
            }
        }
    }
    else {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            if (debug == 1) {
                fprintf(stderr, "PID: %d\n", pid);
                fprintf(stderr, "Executing command: %s\n", pCmdLine->arguments[0]);
            }
            exit(EXIT_FAILURE);
        } else if (pid ==0) {
            // Redirect input if needed
            if (pCmdLine->inputRedirect != NULL) {
                int inputFd;
                if ((inputFd = open(pCmdLine->inputRedirect, O_RDONLY)) == -1) {
                    printf("Input file open error\n");
                    _exit(1);
                } else {
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

        }
        else {
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
    char input[MAX_CMD_LENGTH];
    initHistory();
    cmdLine *parsedCmd;
    process* process_list = NULL;
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
            wakeupProcess(&process_list, pid);
        } else if (strncmp(input, "nuke", 4) == 0) {
            // Parse the process id to terminate
            int pid = atoi(input + 5);
            nukeProcess(&process_list, pid);
        } else if (strncmp(input, "suspend", 7) == 0) {
            // Parse the process id to suspend
            int pid = atoi(input + 8);
            suspendProcess(&process_list, pid);
        } else if (strcmp(input, "procs") == 0){
            procs(&process_list);
        }

        else{
            parsedCmd = parseCmdLines(input);
            if (parsedCmd == NULL){
                fprintf(stderr, "Error parsing command\n");
                continue;
            }

            if (parsedCmd->inputRedirect != NULL) {
                printf("Input redirection from: %s\n", parsedCmd->inputRedirect);
            }
            if (parsedCmd->outputRedirect != NULL) {
                printf("Output redirection to: %s\n", parsedCmd->outputRedirect);
            }

            pid_t pid = fork();
            if (pid == -1){
                perror("fork problem");
                exit(EXIT_FAILURE);
            } else if (pid == 0){
                //child process
                execute(parsedCmd, &process_list);
                freeCmdLines(parsedCmd);
                exit(EXIT_SUCCESS);
            }
            else{
                //parent process
                addProcess(&process_list, parsedCmd, pid);
            }
        }
    }
    //clean
    process* this_process = process_list;
    while (this_process != NULL){
        process* temp = this_process;
        this_process = this_process->next;
        freeCmdLines(temp->cmd);
        free(temp);
    }

    return 0;
}


