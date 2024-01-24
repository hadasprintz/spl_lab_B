#include <stdio.h>
#include <stdlib.h>

void PrintHex(const void *buffer, size_t length) {
    const unsigned char *data = (const unsigned char *)buffer;
    for (size_t i = 0; i < length; ++i) {
        printf("%02X", data[i]);
        if (i < length - 1) {
            printf(" ");
        }
    }
}

int main(int argc, char *argv[]) {
    // Check if the correct number of command-line arguments is provided
    if (argc != 2) {
        fprintf(stderr, "Usage: %s FILE\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the file for reading in binary mode
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Seek to the end of the file to get its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Allocate memory to read the entire file
    unsigned char *buffer = (unsigned char *)malloc(fileSize);
    if (buffer == NULL) {
        perror("Error allocating memory");
        fclose(file);
        return EXIT_FAILURE;
    }

    // Read the entire file into memory
    size_t bytesRead = fread(buffer, 1, fileSize, file);
    if (bytesRead != fileSize) {
        perror("Error reading file");
        free(buffer);
        fclose(file);
        return EXIT_FAILURE;
    }

    // Close the file
    fclose(file);

    // Print the hexadecimal values
    PrintHex(buffer, bytesRead);
    printf("\n");

    // Free allocated memory
    free(buffer);

    return EXIT_SUCCESS;
}
