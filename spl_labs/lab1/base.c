#include <stdlib.h>
#include <stdio.h>
 
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
    return 0;
}







char convertToUpper(char c) {
    if (c >= 'a' && c <= 'z') {
        return c - ('a' - 'A');
    }
    return c; // If not a lowercase letter, return unchanged
}
 
//int main(int argc, char **argv){
////    char* resultArray = map(argv[1], strlen(argv[0]), convertToUpper);
////    printf("%s\n", resultArray);
////    free(resultArray);
////    return 0;
////    printf("%c\n", my_get('a'));
////    char inputChar;
////    printf("Enter a character: ");
////    scanf("%c", &inputChar);
////
////    char result1 = encrypt(inputChar);
////    char result2 = decrypt(inputChar);
////
////
////    printf("Returned value: %c\n", result1);
////    printf("Returned value: %c\n", result2);
////    xprt(inputChar);
//    int base_len = 5;
//    char arr1[base_len];
//    char* arr2 = map(arr1, base_len, my_get);
//    char* arr3 = map(arr2, base_len, cprt);
//    char* arr4 = map(arr3, base_len, xprt);
//    char* arr5 = map(arr4, base_len, encrypt);
//    free(arr2);
//    free(arr3);
//    free(arr4);
//    free(arr5);
//}


