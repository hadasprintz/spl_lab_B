#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>
#include <fcntl.h>

#define MENU_SIZE 7
#define BUFFER_SIZE 128
#define MAX_SECTIONS 100

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
    int mapped_fds[2]; // Array to hold file descriptors for up to two ELF files
    void *mapped_files[2]; // Array to hold mapped memory locations for up to two ELF files
    Elf32_Ehdr *header; // Store ELF header
    Elf32_Shdr *section_headers[MAX_SECTIONS]; // Store section headers
    int num_sections; // Number of sections
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

    s->header = (Elf32_Ehdr *)mapped_file;

    // Verify ELF file
    if (memcmp(s->header->e_ident, ELFMAG, SELFMAG) != 0) {
        printf("Error: Not an ELF file\n");
        munmap(mapped_file, file_size);
        close(fd);
        return;
    }

    // Store file descriptor and mapped memory location
    s->mapped_fds[index] = fd;
    s->mapped_files[index] = mapped_file;

    // Store section header information
    Elf32_Shdr *section_header_start = (Elf32_Shdr *)((uintptr_t)mapped_file + s->header->e_shoff);
    s->num_sections = s->header->e_shnum;
    for (int i = 0; i < s->num_sections; i++) {
        s->section_headers[i] = &section_header_start[i];
    }

    strcpy(s->file_name, file_name); // Update file name in state structure

    if (s->debug_mode) {
        printf("shstrndx: %d\n", s->header->e_shstrndx);
        printf("Section name offset: 0x%08x\n", s->section_headers[s->header->e_shstrndx]->sh_offset);
    }

    // Print magic number bytes
    printf("Magic number bytes 1-3 (ASCII): %c%c%c\n", s->header->e_ident[EI_MAG1], s->header->e_ident[EI_MAG2], s->header->e_ident[EI_MAG3]);

    // Print data encoding scheme
    printf("Data encoding scheme: %s\n", s->header->e_ident[EI_DATA] == ELFDATA2MSB ? "Big Endian" : "Little Endian");

    // Print entry point
    printf("Entry point: 0x%x\n", s->header->e_entry);

    // Print section header table offset, number of entries, and size of each entry
    printf("Section header table offset: 0x%x\n", s->header->e_shoff);
    printf("Number of section header entries: %d\n", s->num_sections);
    printf("Size of each section header entry: %d\n", s->header->e_shentsize);

    // Print program header table offset, number of entries, and size of each entry
    printf("Program header table offset: 0x%x\n", s->header->e_phoff);
    printf("Number of program header entries: %d\n", s->header->e_phnum);
    printf("Size of each program header entry: %d\n", s->header->e_phentsize);
}

void print_section_names(state *s){
    if (s->header == NULL) {
        printf("No ELF file mapped. Please use 'Examine ELF File' option first.\n");
        return;
    }

    printf("Section Names:\n");
    printf("File ELF- %s\n", s->file_name);
    for (int i = 0; i < s->num_sections; i++) {
        Elf32_Shdr *section = s->section_headers[i];
        if(s->debug_mode){
            if (section->sh_type == SHT_SYMTAB || section->sh_type == SHT_DYNSYM) {
                int num_symbols = section->sh_size / sizeof(Elf32_Sym);
                printf("    Symbol Table: Size = %d bytes, Number of Symbols = %d\n", section->sh_size, num_symbols);
            }
        }
        printf("[%d] %s 0x%08x 0x%08x 0x%08x 0x%08x\n", i, (char *)((uintptr_t)s->mapped_files[0] + s->section_headers[s->header->e_shstrndx]->sh_offset + section->sh_name), section->sh_addr, section->sh_offset, section->sh_size, section->sh_type);
    }
}

void print_symbols(state *s){
    printf("Symbols:\n");
    for (int i = 0; i < 2; i++) {
        if (s->mapped_fds[i] != -1) {
            printf("File %s\n", s->file_name);

            Elf32_Shdr *section_header_start = (Elf32_Shdr *)((uintptr_t)s->mapped_files[i] + s->header->e_shoff);
            char *section_name_table = (char *)((uintptr_t)s->mapped_files[i] + section_header_start[s->header->e_shstrndx].sh_offset);

            // Iterate through the section headers to find symbol table sections
            for (int j = 0; j < s->num_sections; j++) {
                Elf32_Shdr *section_header = &section_header_start[j];
                if (section_header->sh_type == SHT_SYMTAB || section_header->sh_type == SHT_DYNSYM) {
                    // Calculate the number of symbols (entries)
                    int num_symbols = section_header->sh_size / sizeof(Elf32_Sym);
                    printf("Symbol Table: Size = %d bytes, Number of Symbols = %d\n", section_header->sh_size, num_symbols);

                    // Print additional debug information if in debug mode
                    if (s->debug_mode) {
                        printf("Symbol Table Index: %d\n", j);
                        printf("Section Name: %s\n", section_name_table + section_header->sh_name);
                    }

                    // Get the symbol table pointer
                    Elf32_Sym *symbol_table = (Elf32_Sym *)((uintptr_t)s->mapped_files[i] + section_header->sh_offset);

                    // Get the string table pointer
                    Elf32_Shdr *strtab_section_header = &section_header_start[section_header->sh_link];
                    char *str_table = (char *)((uintptr_t)s->mapped_files[i] + strtab_section_header->sh_offset);

                    // Iterate through the symbols and print their information
                    for (int k = 0; k < num_symbols; k++) {
                        Elf32_Sym *symbol = &symbol_table[k];
                        char *section_name = "Undefined";
                        if (symbol->st_shndx != SHN_UNDEF && symbol->st_shndx < s->num_sections) {
                            section_name = section_name_table + section_header_start[symbol->st_shndx].sh_name;
                        }
                        printf("[%d] %08x %d %s %s\n", k, symbol->st_value, symbol->st_shndx, section_name, str_table + symbol->st_name);
                    }
                }
            }
        }
    }
}

void check_files_for_merge(state *s){
    printf("Not implemented yet.\n");
}

void merge_ELF_files(state *s){
    // Check if two ELF files are opened
    if (s->mapped_fds[0] == -1 || s->mapped_fds[1] == -1) {
        printf("Error: Two ELF files must be opened to perform merging\n");
        return;
    }

    // Open output file for writing
    int out_fd = open("output.ro", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (out_fd == -1) {
        perror("Error opening output file");
        return;
    }

    // Initialize ELF header structures for both files
    Elf32_Ehdr *header1 = (Elf32_Ehdr *)s->mapped_files[0];
    Elf32_Ehdr *header2 = (Elf32_Ehdr *)s->mapped_files[1];

    // Copy header from the first ELF file to the output file
    if (write(out_fd, header1, sizeof(Elf32_Ehdr)) == -1) {
        perror("Error writing ELF header");
        close(out_fd);
        return;
    }

    // Merge section headers
    Elf32_Shdr *section_headers1 = (Elf32_Shdr *)(s->mapped_files[0] + header1->e_shoff);
    Elf32_Shdr *section_headers2 = (Elf32_Shdr *)(s->mapped_files[1] + header2->e_shoff);

    size_t num_sections1 = header1->e_shnum;
    size_t num_sections2 = header2->e_shnum;

    // Calculate size of section headers for each file
    size_t section_headers_size1 = num_sections1 * sizeof(Elf32_Shdr);
    size_t section_headers_size2 = num_sections2 * sizeof(Elf32_Shdr);

    // Write section headers from the first ELF file to the output file
    if (write(out_fd, section_headers1, section_headers_size1) == -1) {
        perror("Error writing section headers from file 1");
        close(out_fd);
        return;
    }

    // Write section headers from the second ELF file to the output file
    if (write(out_fd, section_headers2, section_headers_size2) == -1) {
        perror("Error writing section headers from file 2");
        close(out_fd);
        return;
    }

    // Merge section contents (skipping symbol table sections)
    for (size_t i = 0; i < num_sections1; i++) {
        // Skip symbol table sections
        if (section_headers1[i].sh_type == SHT_SYMTAB || section_headers1[i].sh_type == SHT_DYNSYM)
            continue;

        // Write section contents from the first ELF file to the output file
        if (write(out_fd, s->mapped_files[0] + section_headers1[i].sh_offset, section_headers1[i].sh_size) == -1) {
            perror("Error writing section contents from file 1");
            close(out_fd);
            return;
        }
    }

    for (size_t i = 0; i < num_sections2; i++) {
        // Skip symbol table sections
        if (section_headers2[i].sh_type == SHT_SYMTAB || section_headers2[i].sh_type == SHT_DYNSYM)
            continue;

        // Write section contents from the second ELF file to the output file
        if (write(out_fd, s->mapped_files[1] + section_headers2[i].sh_offset, section_headers2[i].sh_size) == -1) {
            perror("Error writing section contents from file 2");
            close(out_fd);
            return;
        }
    }

    // Close output file
    close(out_fd);

    printf("Merging completed successfully. Output written to output.ro\n");

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
    s.header = NULL;


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