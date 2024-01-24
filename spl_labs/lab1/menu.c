//
// Created by hadas on 1/10/24.
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct fun_desc{
    char *name;
    char (*fun)(char);
};

char* map(char *array, int array_length, char (*f) (char)){
    char* mapped_array = (char*)(malloc(array_length*sizeof(char)));
    for (int i = 0; i < array_length; i++) {
        mapped_array[i] = f(array[i]);
    }
    free(array);
    return mapped_array;
}

char my_get(char c){
    c = fgetc(stdin);
    return (char) c;
}

char cprt(char c){
    if (c >= 0x20 && c <= 0x7E) {
        printf("%c\n", c);
    } else {
        printf(".\n");
    }
    return c;
}

char encrypt(char c){
    if (c >= 0x20 && c <= 0x7E) {
        return c +1;
    }
    return c;
}

char decrypt(char c){
    if (c >= 0x20 && c <= 0x7E) {
        return c -1;
    }
    return c;
}

char xprt(char c) {
    // Print the value of c in hexadecimal representation followed by a new line
    printf("%02X\n", c);

    // Return c unchanged
    return c;
}


int main(int argc, char **argv){
    struct fun_desc menu[] = {
            { "Get string", my_get },
            { "Print string", cprt },
            { "Encrypt", encrypt },
            { "Decrypt", decrypt },
            { "Print Hex", xprt },
            {NULL, NULL}
    };

    char* carray = calloc(5, sizeof(char));
    while (1) {
        // Print menu
        printf("\nPlease choose a function (ctrl^D for exit):\n");
        for (int i = 0; i < 5; i++) {
            printf("%d) %s\n", i, menu[i].name);
        }

        // Read input line from stdin
        char input[256]; // Adjust the array size as needed
        if (fgets(input, sizeof(input), stdin) == NULL) {
            // If fgets encounters an EOF condition, exit the loop
            break;
        }

        int choice = atoi(input);

        printf("Option :  %s", input);

        if (0 <= choice && choice <= 4){
            printf("Within bounds\n");
        }
        else{
            printf("Not Within bounds\n");
            free(carray);
            return 1;
        }

        // Print the input

        carray = map(carray, 5, menu[choice].fun);
        printf("DONE.\n");
    }
        free(carray);
        return 0;
}