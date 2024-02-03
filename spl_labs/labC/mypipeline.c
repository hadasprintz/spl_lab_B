#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv){
    int pip_file_desc[2];
    if (pipe(pip_file_desc) == -1){
        perror("error in pipe");
        exit(EXIT_FAILURE);
    }
    fprintf(stderr, "parent_process>forking…\n");
    pid_t child1 = fork();
    fprintf(stderr, "parent_process>created process with id: %d\n", child1);
    if (child1 == -1) {
        perror("Error in fork");
        exit(EXIT_FAILURE);
    }
    //child 1 process
    if (child1 == 0){
        close(1);
        dup(pip_file_desc[1]);
        close(pip_file_desc[1]);
        char *ls_args[] = {"ls", "-l", NULL};
        execvp("ls", ls_args);
    }
    //parent process
    else{
        fprintf(stderr, "parent_process>closing the write end of the pipe…\n");
        close(pip_file_desc[1]);
        pid_t child2 = fork();
        fprintf(stderr, "parent_process>created process with id: %d\n", child2);
        if (child2 == -1) {
            perror("Error in fork");
            exit(EXIT_FAILURE);
        }
        //child 2 process
        if (child2 == 0){
            close(0);
            dup(pip_file_desc[0]);
            close(pip_file_desc[0]);
            char *tail_args[] = {"tail", "-n", "2", NULL};
            execvp("tail", tail_args);
        }
        //parent process
        else{
            fprintf(stderr, "parent_process>closing the read end of the pipe…\n");
            close(pip_file_desc[0]);
            fprintf(stderr, "parent_process>waiting for child processes to terminate…\n");
            waitpid(child1, NULL, 0);
            waitpid(child2, NULL, 0);
            fprintf(stderr, "(parent_process>exiting...)\n");
        }
    }

    return 0;


}