#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
    int pip_file_desc[2];

    if (pipe(pip_file_desc) == -1){
        perror("error in pipe");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        char* m = "hello";
        //write to the parent - pip_file_desc[1]
        if (write(pip_file_desc[1], m, strlen(m)) == -1){
            perror("error in write");
            exit(EXIT_FAILURE);
        }
        close(pip_file_desc[0]);
        close(pip_file_desc[1]);
    }
    else{
        char buffer[512];
        ssize_t read_output;
        if ((read_output = read(pip_file_desc[0], buffer, 511)) == -1){
            perror("error in read");
            exit(EXIT_FAILURE);
        }
        buffer[read_output] = '\0';
        printf("The message is: %s\n", buffer);

        close(pip_file_desc[0]);
        close(pip_file_desc[1]);
    }
    return 0;
}