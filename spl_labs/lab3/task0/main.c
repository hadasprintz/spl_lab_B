#include "util.h"

#define SYS_WRITE 4
#define STDOUT 1
#define SYS_EXIT 1
#define SYS_OPEN 5
#define O_RDWR 2
#define SYS_SEEK 19
#define SEEK_SET 0
#define SHIRA_OFFSET 0x291

extern int system_call();

int main (int argc , char* argv[], char* envp[])
{
    int i = 0; /* Counter for arguments */
    while (i < argc) {
        const char *arg = argv[i];
        int length = strlen(arg);
        system_call(SYS_WRITE, STDOUT, arg, length);
        system_call(SYS_WRITE, STDOUT, "\n", 1);
        i++;
    }

    return 0;
}

