#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MENU_SIZE 9

typedef struct {
    char debug_mode;
    char file_name[128];
    int unit_size;
    unsigned char mem_buf[10000];
    size_t mem_count;
    char display_mode;
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

void set_file_name(state *s) {
    printf("Enter file name: ");
    fgets(s->file_name, sizeof(s->file_name), stdin);
    // Remove trailing newline if present
    s->file_name[strcspn(s->file_name, "\n")] = '\0';

    if (s->debug_mode) {
        fprintf(stderr, "Debug: file name set to '%s'\n", s->file_name);
    }
}

void set_unit_size(state *s) {
    int size;
    printf("Enter unit size (1, 2, or 4): ");
    scanf("%d", &size);
    getchar(); // Consume newline

    if (size == 1 || size == 2 || size == 4) {
        s->unit_size = size;
        if (s->debug_mode) {
            printf("Debug: set size to %d\n", s->unit_size);
        }
    } else {
        printf("Error: Invalid unit size\n");
    }
}

void quit(state *s) {
    if (s->debug_mode) {
        printf("Quitting\n");
    }
    exit(0);
}

void load_Into_Memory(state *s){
    if (strcmp(s->file_name, "") == 0) {
        printf("Error: File name is not set\n");
        return;
    }

    FILE *file = fopen(s->file_name, "rb");
    if (file == NULL) {
        printf("Error: Failed to open file for reading\n");
        return;
    }

    unsigned int location;
    size_t length;

    printf("Please enter <location> <length>\n");
    scanf("%X %zu", &location, &length);
    getchar(); // Consume newline

    if (s->debug_mode) {
        printf("Debug: Loading %zu units into memory from file '%s' starting at location %X\n", length, s->file_name, location);
    }

    fseek(file, location, SEEK_SET);
    fread(s->mem_buf, s->unit_size, length, file);
    s->mem_count = length;

    printf("Loaded %zu units into memory\n", length);

    fclose(file);
}

void toggle_display_mode(state *s){
    if (s->display_mode) {
        printf("Display flag now off, decimal representation\n");
        s->display_mode = 0;
    } else {
        printf("Display flag now on, hexadecimal representation\n");
        s->display_mode = 1;
    }
}

void memory_display(state *s){
    size_t addr, length;
    printf("Enter address and length: ");
    scanf("%zx %zu", &addr, &length);
    getchar();

    printf("%s\n", s->display_mode ? "Hexadecimal" : "Decimal");
    printf("%s\n", s->display_mode ? "===========" : "=======");

    for (size_t i = 0; i < length; ++i) {
        unsigned int val = 0;
        memcpy(&val, s->mem_buf + addr + i * s->unit_size, s->unit_size);

        if (s->display_mode ) {
            printf("%#x\n", val);
        } else {
            printf("%zu\n", val);
        }
    }
}

void save_into_file(state *s){
    // Check if file_name is empty
    if (strlen(s->file_name) == 0) {
        fprintf(stderr, "Error: File name is empty.\n");
        return;
    }

    FILE* file = fopen(s->file_name, "r+b"); // Open for read/write without truncating
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file for writing.\n");
        return;
    }

    // Query the user for source address, target location, and length
    size_t source_address, target_location, length;
    printf("Please enter <source-address> <target-location> <length>\n");
    scanf("%zx %zx %zu", &source_address, &target_location, &length);

    // Check if target location is valid
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    if (target_location + length > file_size) {
        fprintf(stderr, "Error: Target location exceeds file size.\n");
        fclose(file);
        return;
    }

    // Print debug messages if debug mode is on
    if (s->debug_mode) {
        printf("Debug: File name: '%s', Source address: %#zx, Target location: %#zx, Length: %zu\n", s->file_name, source_address, target_location, length);
    }

    // Copy data from memory to file
    fseek(file,0,SEEK_SET); //back to start of file
    fseek(file,target_location,SEEK_SET);
    if(source_address==0){
        fwrite(&(s->mem_buf),s->unit_size,length,file);
        s->mem_count = length;
    }
    else{
        fwrite(&(source_address),s->unit_size,length,file);
        s->mem_count = length;
    }

    // Close the file
    fclose(file);
}

void memory_modify(state *s){
    if (s->mem_count == 0) {
        fprintf(stderr, "Error: Memory is empty. Please load data into memory first.\n");
        return;
    }

    size_t location, val;
    printf("Please enter <location> <val>: ");
    scanf("%zx %zx", &location, &val);
    getchar();

    if (s->debug_mode) {
        printf("Debug: Location: %#zx, Val: %#zx\n", location, val);
    }

    size_t max_location = s->mem_count / s->unit_size;
    if (location >= max_location) {
        fprintf(stderr, "Error: Invalid location. Location exceeds memory size.\n");
        return;
    }

    // Convert val to the appropriate size based on unit_size
    if (s->unit_size == 1) {
        unsigned char value = val;
        memcpy(s->mem_buf + location, &value, 1);
    } else if (s->unit_size == 2) {
        unsigned short value = val;
        memcpy(s->mem_buf + location * 2, &value, 2);
    } else if (s->unit_size == 4) {
        unsigned int value = val;
        memcpy(s->mem_buf + location * 4, &value, 4);
    }

    // Write the modified memory back to the file
    FILE *file = fopen(s->file_name, "rb+");
    if (file == NULL) {
        fprintf(stderr, "Error: Unable to open file for writing.\n");
        return;
    }

    fseek(file, location * s->unit_size, SEEK_SET);
    fwrite(s->mem_buf + location * s->unit_size, s->unit_size, 1, file);
    fclose(file);

    printf("Modified memory at location %#zx\n", location);
}


int main() {
    state s = {0}; // Initialize all members to 0

    void (*menu[MENU_SIZE])(state *s) = {
            toggle_debug_mode,
            set_file_name,
            set_unit_size,
            load_Into_Memory, // Load Into Memory
            toggle_display_mode, // Toggle Display Mode
            memory_display, // Memory Display
            save_into_file, // Save Into File
            memory_modify, // Memory Modify
            quit
    };

    char *menu_names[MENU_SIZE] = {
            "Toggle Debug Mode",
            "Set File Name",
            "Set Unit Size",
            "Load Into Memory",
            "Toggle Display Mode",
            "Memory Display",
            "Save Into File",
            "Memory Modify",
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