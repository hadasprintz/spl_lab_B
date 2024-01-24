#include <stdlib.h>
#include <stdio.h>
#include <string.h>


int main(int argc, char **argv) {
    int debug_mode = 1;
    FILE *input_file = stdin;
    FILE *output_file = stdout;
    char *encrypt;
    for (int i = 1; i < argc; i++) {
        if (debug_mode == 1) {
            fprintf(stderr, "%s\n", argv[i]);
        }
        if (strcmp(argv[i], "+D") == 0) {
            debug_mode = 1;
        } else if (strcmp(argv[i], "-D") == 0) {
            debug_mode = 0;
        }
        if (strncmp(argv[i], "+E", 2) == 0 || strncmp(argv[i], "-E", 2) == 0){
            encrypt = argv[i];
        }
        if (strncmp(argv[i], "-I", 2) == 0){
            input_file = fopen(argv[i] +2, "r");
            if (input_file == NULL){
                fprintf(stderr, "Error opening input file\n");
                return 1;
            }
        }
        if (strncmp(argv[i], "-O", 2) == 0){
            output_file = fopen(argv[i] +2, "w");
            if (output_file == NULL){
                fprintf(stderr, "Error opening output file\n");
                return 1;
            }
        }
    }
    encode(encrypt, input_file, output_file);
    return 0;
}

void encode(char *encryption_key, FILE *file_input, FILE *file_output){
    int enc_key = 0;
    int encoding_length = 0;
    while (encryption_key[encoding_length] != '\0') {
        encoding_length++;
    }
    encoding_length = encoding_length - 2;
    int encoding_index = 0;
    int c;
    if (encryption_key[0] == '+'){
        enc_key = 1;
    }
    else if (encryption_key[0] == '-'){
        enc_key = 0;
    }
    while ((c = fgetc(file_input)) != EOF){
        int key = encryption_key[encoding_index + 2];
        if (enc_key == 1){
            key = key - '0';
            if (c >= '0' && c <= '9'){
                c += key;
                if (c > '9'){
                    c -= 10;
                }

            }
            if (c >= 'A' && c <= 'Z'){
                c += key;
                if (c > 'Z') {
                    c -= 26;
                }

            }

        }
        else{
            key = key - '0';
            if (c >= '0' && c <= '9'){
                c -= key;
                if (c < '0'){
                    c += 10;
                }

            }
            if (c >= 'A' && c <= 'Z'){
                c -= key;
                if (c < 'A'){
                    c += 26;
                }

            }

        }
        fputc((char) c, file_output);
        encoding_index = (encoding_index + 1) % encoding_length;

    }

}