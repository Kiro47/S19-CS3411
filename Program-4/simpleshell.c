#include "utils.h"
#include "shell.h"

int main(int argc, char **argv)
{
    int shellCode;

    /*
    * Do the shell stuff
    */
    while(1)
    {
        shellCode = spawnShell(argv[0], shellCode);
        if (shellCode == 1)
        {
            // Shell returned `exit`
            return 0;
        }
    }
}
