#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

int format = 0;

// Define the virus struct
typedef struct virus {
    unsigned short SigSize;
    char virusName[16];
    unsigned char* sig;
} virus;

typedef struct link link;

struct link {
    link *nextVirus;
    virus *vir;
};


void PrintHex(FILE* output, unsigned char *buffer, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        fprintf(output, "%02hhX ", buffer[i]);
    }
    fprintf(output, "\n\n");
}

// Function to read a virus from a file
virus* readVirus(FILE* file) {
    virus* newVirus = malloc(sizeof(struct virus));
    if(fread(newVirus, sizeof(unsigned short), 1, file) != 1) {
        free(newVirus);
        return NULL;
    }

    if (format == 0){
        newVirus->SigSize = ntohs(newVirus->SigSize);
    }
    else{
        newVirus->SigSize = newVirus->SigSize;
    }
    newVirus->sig = malloc(newVirus->SigSize);
    if (fread(newVirus->virusName, sizeof(char), 16, file) != 16 ||
        fread(newVirus->sig, 1, newVirus->SigSize, file) != newVirus->SigSize) {
        free(newVirus->sig);
        free(newVirus);
        return NULL;
    }
    return newVirus;
}

void printVirus(virus* virus, FILE* output) {
    fprintf(output, "Virus name: %s \n", virus->virusName);
    fprintf(output, "Virus size: %u \n", virus->SigSize);
    printf("signature: \n");
    PrintHex(output, virus->sig, virus->SigSize);
    fprintf(output, "\n");
}

void list_print(link *virus_list, FILE* output) {
    link *current = virus_list;
    while (current != NULL) {
        printVirus(current->vir, output);
        current = current->nextVirus;
    }
}

link* newLink(virus* data){
    link* newLink = malloc(sizeof(struct link));
    newLink->vir=data;
    newLink->nextVirus=NULL;
    return newLink;
}

link* list_append(link* virus_list, virus* data) {
    if (virus_list == NULL)
        virus_list = newLink(data);
    else
        virus_list->nextVirus = list_append(virus_list->nextVirus,data);
    return virus_list;
}

void list_free(link *virus_list) {
    link *current = virus_list;
    while (current != NULL) {
        link *next = current->nextVirus;
        free(current->vir->sig);
        free(current->vir);
        free(current);
        current = next;
    }
}

int getSize(FILE* file) {
    fseek(file, 0L, SEEK_END);
    int fileSize = ftell(file);
    rewind(file);     //sets the file position to the beginning
    return fileSize;
}

link* load_list(FILE* file){
    link* head=NULL;
    int fileSize = getSize(file);
    char buffer[4];
    fread(&buffer, 1, 4, file);
    int bytes = 4;       //readen bytes
    while(bytes<fileSize){
        virus* nextVirus = readVirus(file);
        if (nextVirus == NULL){
            break;
        }
        head=list_append(head,nextVirus);
        bytes=bytes+18+nextVirus->SigSize;
    }
    fclose(file);
    return head;
}

link* load_signatures(link* link) {
    char* filename = NULL;
    char buf[BUFSIZ];

    printf("Enter signature file name:\n");
    fgets(buf,sizeof(buf),stdin);
    sscanf(buf,"%ms",&filename);
    FILE* file = fopen(filename,"rb");
    char magic[5];
    fread(magic, sizeof(char), 4, file);
    magic[4] = '\0';
    if (strcmp(magic, "VIRL") != 0 && strcmp(magic, "VIRB") != 0) {
        fprintf(stderr, "Error: Incorrect magic number in the signatures file.\n");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Print an informative message based on the magic number
    if (strcmp(magic, "VIRL") == 0) {
        format = 1;
    }
    free(filename);
    if(file==NULL){
        fprintf(stderr,"Reading File Error\n");
        exit(EXIT_FAILURE);
    }
    return load_list(file);
}

void detect_virus(char *buffer, unsigned int size, link *virus_list) {
    link *current = virus_list;
    while (current != NULL) {
        virus *currentVirus = current->vir;
        size_t i = 0;
        while (i <= size - currentVirus->SigSize) {
            if (memcmp(buffer + i, currentVirus->sig, currentVirus->SigSize) == 0) {
                printf("Virus detected at byte %zu:\n", i);
                printf("Virus name: %s\n", currentVirus->virusName);
                printf("Virus size: %u\n\n", currentVirus->SigSize);
            }
            ++i;
        }
        current = current->nextVirus;
    }
}

link* detect(FILE *file, char *buffer, unsigned int size, link *virus_list){
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }
    fseek(file, 0L, SEEK_END);
    unsigned int fileSize = ftell(file);
    rewind(file);
    if(fread(buffer, 1, fileSize, file) != fileSize){
        perror("Error reading file");
        fclose(file);
        free(buffer);
        exit(EXIT_FAILURE);
    }
    fclose(file);

    detect_virus(buffer, size, virus_list);
    return virus_list;
}

void neutralize_virus(FILE *file, int signatureOffset, int signaturSize) {
    fseek(file, signatureOffset, SEEK_SET);
    // Replace the first byte with the RET instruction (0xC3)
    char *change = malloc(signaturSize);
    for(int i=0;i<signaturSize;i++)
        change[i]= 0xC3;
    fwrite(change,1,signaturSize,file);
    free(change);
}

void fix_file(const char *filename, link *virus_link) {
    FILE *file = fopen(filename, "r+b");  // Open the file in read/write binary mode
    if (file == NULL) {
        perror("Error opening file for fixing");
        return;
    }
    fseek(file, 0L, SEEK_END);
    unsigned int fileSize = ftell(file);
    rewind(file);

    char *buffer = (char *) malloc(fileSize);
    if (fread(buffer, 1, fileSize, file) != fileSize) {
        perror("Error reading file for fixing");
        fclose(file);
        free(buffer);
        return;
    }

    link *current = virus_link;
    while (current != NULL) {
        virus *currentVirus = current->vir;
        size_t i = 0;
        while (i <= fileSize - currentVirus->SigSize) {
            if (memcmp(buffer + i, currentVirus->sig, currentVirus->SigSize) == 0) {
                neutralize_virus(file, i, currentVirus->SigSize);
            }
            ++i;
        }
        current = current->nextVirus;
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <signatures_file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Open the signatures file
    FILE *file = fopen(argv[1], "rb");
    if (file == NULL) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    // Check the magic number

    link *virus_list = NULL;
    char choice;
    char* filename = malloc(BUFSIZ);
    char buffer[10240]; // 10K buffer size
    size_t bytesRead = fread(buffer, 1, sizeof(buffer), file);
    while (1) {
        printf("\n1) Load signatures\n2) Print signatures\n3) Detect viruses\n4) Fix file\n5) Quit\n");
        printf("Enter your choice: ");
        fgets(filename, sizeof(filename), stdin);

        if (sscanf(filename, "%c", &choice) != 1) {
            fprintf(stderr, "Invalid input.\n");
            continue;
        }

        switch (choice) {
            case '1':
                virus_list = load_signatures(virus_list);
                break;
            case '2':
                list_print(virus_list, stdout);
                break;
            case '3':
                virus_list = detect(file, buffer, bytesRead, virus_list);
                break;
            case '4':
                fix_file(argv[1], virus_list);
                break;
            case '5':
                list_free(virus_list);
                printf("Exiting program.\n");
                free(filename);
                exit(EXIT_SUCCESS);
            default:
                printf("Invalid choice. Try again.\n");
        }
    }
    free(filename);
    return 0;
}
