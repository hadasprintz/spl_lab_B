#include <stdio.h>

int digit_cnt(char *str) {
    int count = 0;
    while (*str) {
        if (*str >= '0' && *str <= '9') {
            count++;
        }
        str++;
    }
    return count;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Usage: %s <string>\n", argv[0]);
        return 1;
    }
    printf("The number of digits in the string is: %d\n", digit_cnt(argv[1]));
    return 0;
}
