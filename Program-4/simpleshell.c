#include "utils.h"
#include "shell.h"

#include <limits.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
    int* shellCode;
    char* processName;
    int allocation;

    allocation = strlen(argv[0]) + 1;
    processName = malloc(allocation);
    strcpy(processName, argv[0]);
    shellCode = malloc(sizeof(int));
    shellCode[0] = 0;
    /*
    * Do the shell stuff
    */
    while(1)
    {
        spawnShell(processName, shellCode);
        if (shellCode[0] == INT_MIN)
        {
            // Shell returned `exit`
            free(processName);
            free(shellCode);
            return 0;
        }
    }
}
