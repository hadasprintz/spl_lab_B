#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>
#include <unistd.h>

typedef struct {
    void *map_start;
    void (*func)(Elf32_Phdr *, int);
    int arg;
} PhdrIteratorArgs;

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg);

void process_phdr(Elf32_Phdr *phdr, int arg);

void load_phdr(Elf32_Phdr *phdr, int fd);

int startup(int argc, char **argv, void (*start)());

int foreach_phdr(void *map_start, void (*func)(Elf32_Phdr *, int), int arg) {
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    Elf32_Phdr *phdr = (Elf32_Phdr *)((char *)map_start + elf_header->e_phoff);
    int phnum = elf_header->e_phnum;

    for (int i = 0; i < phnum; ++i) {
        func(phdr + i, arg);
    }

    return 0;
}

void process_phdr(Elf32_Phdr *phdr, int arg) {
    printf("%-6s 0x%06x 0x%08x 0x%08x 0x%05x 0x%05x %-2s%-2s 0x%x\n",
           (phdr->p_type == PT_PHDR) ? "PHDR" :
           (phdr->p_type == PT_INTERP) ? "INTERP" :
           (phdr->p_type == PT_LOAD) ? "LOAD" :
           (phdr->p_type == PT_DYNAMIC) ? "DYNAMIC" :
           (phdr->p_type == PT_NOTE) ? "NOTE" : "UNKNOWN",
           phdr->p_offset,
           phdr->p_vaddr,
           phdr->p_paddr,
           phdr->p_filesz,
           phdr->p_memsz,
           (phdr->p_flags & PF_R) ? "R" : " ",
           (phdr->p_flags & PF_W) ? "W" : " ",
           phdr->p_align);
    // Print protection flags
    printf("%s%s%s",
           (phdr->p_flags & PF_R) ? "PROT_READ " : "",
           (phdr->p_flags & PF_W) ? "PROT_WRITE " : "",
           (phdr->p_flags & PF_X) ? "PROT_EXEC " : "");

    // Print mapping flags
    printf("%s%s\n",
           (phdr->p_flags & PF_X) ? "MAP_EXECUTABLE " : "MAP_FIXED ",
           "MAP_PRIVATE");

}

void load_phdr(Elf32_Phdr *phdr, int fd) {
    // Check if the program header is of type PT_LOAD
    if (phdr->p_type != PT_LOAD) {
        return;
    }

    // Calculate the aligned starting virtual address
    void *vaddr = (void *)(phdr->p_vaddr & 0xFFFFF000);

    // Calculate the aligned file offset
    off_t offset = phdr->p_offset & 0xFFFFF000;

    // Calculate the padding (difference between actual and aligned virtual address)
    size_t padding = phdr->p_vaddr & 0xfff;

    // Map the program header into memory
    void *map = mmap(vaddr, phdr->p_memsz + padding, phdr->p_flags, MAP_PRIVATE | MAP_FIXED, fd, offset);

    if (map == MAP_FAILED) {
        perror("Error mapping program header to memory");
        return;
    }

    // Print information about the mapped program header
    process_phdr(phdr, 0);
}



int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ELF file>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("Error opening file");
        return EXIT_FAILURE;
    }

    off_t file_size = lseek(fd, 0, SEEK_END);
    void *map_start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);

    if (map_start == MAP_FAILED) {
        perror("Error mapping file to memory");
        close(fd);
        return EXIT_FAILURE;
    }

    foreach_phdr(map_start, load_phdr, fd);

    // Call startup function to transfer control to the loaded program
    Elf32_Ehdr *elf_header = (Elf32_Ehdr *)map_start;
    startup(argc - 1, argv + 1, (void *)(elf_header->e_entry));


    close(fd);
    munmap(map_start, file_size);

    return EXIT_SUCCESS;
}


