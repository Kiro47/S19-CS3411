#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
// Included for snprintf and vsnprintf
// Explicitly asked about this.
#include <stdio.h>


/* These would normaly be part of their own Lib, but can't submit them */
/* ------------------------------------------------------------------------ */
/* FUNCTION  print :                                                        */
/*     Checks if a spot in memory is free, if not free i                    */
/* PARAMETER USAGE :                                                        */
/*     allocation : Allocated memory to free                                */
/* ------------------------------------------------------------------------ */
char* checkFree(char *allocation)
{
    if (allocation != NULL)
    {
        free(allocation);
        allocation = NULL;
    }
    return NULL;
}

/* ------------------------------------------------------------------------ */
/* FUNCTION  print :                                                        */
/*     Prints the given message and dynamically                             */
/*     allocates required memory for it.                                    */
/* PARAMETER USAGE :                                                        */
/*     msg: The message to print to buffer, uses printf syntax              */
/* ------------------------------------------------------------------------ */
void print(char *fmt, ...)
{
    int actualRoom;
    int allocatedRoom = 32; // Pre allocate estimated lenght
    char *buffer, *tempBuffer;
    va_list args;
    buffer = NULL;
    tempBuffer = NULL;
    actualRoom = 0;
    if ((buffer = malloc(allocatedRoom)) == NULL)
    {
        return;
    }

    while (1) {

        /* Try to print in the allocated space */
        va_start(args, fmt);
        actualRoom = vsnprintf(buffer, allocatedRoom, fmt, args);
        va_end(args);

        /* Check error code */

        if (actualRoom < 0)
        {
            return;
        }

        /* If that worked, return the string */

        if (actualRoom < allocatedRoom)
        {
            break;
        }

        /* Else try again with more space */

        allocatedRoom = actualRoom + 1;       /* Precisely what is needed */


        if ((tempBuffer = realloc (buffer, allocatedRoom)) == NULL)
        {
            free(buffer);
            return;
        } else
        {
            buffer = tempBuffer;
            free(tempBuffer);
        }
    }

	write(1, buffer, strlen(buffer));
	free(buffer);
}










void getChar(int stdinFD, char* character)
{
    int count;
    count = read(stdinFD, &character, sizeof(char));

    if (count > SSIZE_MAX)
    {
        // Undefined behavior, bail hard.
        print("Undefinable behavior occured getting character.\n");
        return; // SCHAR_MIN from limits
    }

    if (count < 0)
    {
        // Get error stuff
        print(strerror(errno));
    }
    print("%c\n", *character);
}
