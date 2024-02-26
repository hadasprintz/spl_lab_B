#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>

#define MENU_SIZE 7
#define BUFFER_SIZE 128

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
    int mapped_fds[2]; // Array to hold file descriptors for up to two ELF files
    void *mapped_files[2]; // Array to hold mapped memory locations for up to two ELF files
} state;

void toggle_debug_mode(state *s) {
    if (s->debug_mode) {
        printf("Debug flag now off\n");
        s->debug_mode = 0;
    } else {
        printf("Debug flag now on\n");
        s->debug_mode = 1;
    }
}

void examine_ELF_file(state *s){
    char file_name[BUFFER_SIZE];
    printf("Enter ELF file name: ");
    fgets(file_name, BUFFER_SIZE, stdin);
    file_name[strcspn(file_name, "\n")] = '\0'; // Remove trailing newline

    int index = -1;
    for (int i = 0; i < 2; i++) {
        if (s->mapped_fds[i] == -1) {
            index = i;
            break;
        }
    }

    if (index == -1) {
        printf("Cannot open more than 2 ELF files simultaneously\n");
        return;
    }

    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return;
    }

    // Obtain file size
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size == -1) {
        perror("Error getting file size");
        close(fd);
        return;
    }

    void *mapped_file = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped_file == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(fd);
        return;
    }

    Elf32_Ehdr *header = (Elf32_Ehdr *)mapped_file;

    // Verify ELF file
    if (memcmp(header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not an ELF file\n");
        munmap(mapped_file, file_size);
        close(fd);
        return;
    }

    // Store file descriptor and mapped memory location
    s->mapped_fds[index] = fd;
    s->mapped_files[index] = mapped_file;

    // Print magic number bytes
    printf("Magic number bytes 1-3 (ASCII): %c%c%c\n", header->e_ident[EI_MAG1], header->e_ident[EI_MAG2], header->e_ident[EI_MAG3]);

    // Print data encoding scheme
    printf("Data encoding scheme: %s\n", header->e_ident[EI_DATA] == ELFDATA2MSB ? "Big Endian" : "Little Endian");

    // Print entry point
    printf("Entry point: 0x%x\n", header->e_entry);

    // Print section header table offset, number of entries, and size of each entry
    printf("Section header table offset: 0x%x\n", header->e_shoff);
    printf("Number of section header entries: %d\n", header->e_shnum);
    printf("Size of each section header entry: %d\n", header->e_shentsize);

    // Print program header table offset, number of entries, and size of each entry
    printf("Program header table offset: 0x%x\n", header->e_phoff);
    printf("Number of program header entries: %d\n", header->e_phnum);
    printf("Size of each program header entry: %d\n", header->e_phentsize);

    // Unmap the file from memory
    munmap(mapped_file, file_size);
    close(fd);
}

void print_section_names(state *s){
    printf("Not implemented yet.\n");
}

void print_symbols(state *s){
    printf("Not implemented yet.\n");
}

void check_files_for_merge(state *s){
    printf("Not implemented yet.\n");
}

void merge_ELF_files(state *s){
    printf("Not implemented yet.\n");
}


void quit(state *s) {
    if (s->debug_mode) {
        printf("Quitting\n");
    }
    for (int i = 0; i < 2; i++) {
        if(s->mapped_fds[i] != -1){
            munmap(s->mapped_files[i], s->mem_count);
            close(s->mapped_fds[i]);
            printf("File %d: Mapped file unmapped and closed.\n", i + 1);
        }
    }
    printf("Exiting...\n");
    exit(0);
}


int main() {
    state s = {0}; // Initialize all members to 0
    s.unit_size = 1;
    s.mapped_fds[0] = -1; // Initialize file descriptors to -1
    s.mapped_fds[1] = -1;

    s.mapped_files[0] = NULL; // Initialize mapped memory locations to NULL
    s.mapped_files[1] = NULL;


    void (*menu[MENU_SIZE])(state *s) = {
            toggle_debug_mode,
            examine_ELF_file,
            print_section_names,
            print_symbols,
            check_files_for_merge,
            merge_ELF_files,
            quit
    };

    char *menu_names[MENU_SIZE] = {
            "Toggle Debug Mode",
            "Examine Elf File",
            "Print Section Names",
            "Print Symbols",
            "Check Files For Merge",
            "Merge ELF Files",
            "Quit"
    };

    while (1) {
        if (s.debug_mode) {
            printf("Debug: unit_size = %d, file_name = '%s', mem_count = %zu\n", s.unit_size, s.file_name, s.mem_count);
        }

        printf("\nChoose action:\n");
        for (int i = 0; i < MENU_SIZE; i++) {
            printf("%d-%s\n", i, menu_names[i]);
        }

        int choice;
        printf("\nEnter choice: ");
        scanf("%d", &choice);
        getchar(); // Consume newline

        if (choice >= 0 && choice < MENU_SIZE) {
            menu[choice](&s);
        } else {
            printf("Invalid choice\n");
        }
    }

    return 0;
}