#include <stddef.h>
#include <sys/types.h>
#include <errno.h>

// Included for snprintf and vsnprintf
// Explicitly asked about this.
#include <stdio.h>


// Maybe??? Ask
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
/* FUNCTION  print :                                                        */
/*     Prints the given message and dynamically                             */
/*     allocates required memory for it.                                    */
/* PARAMETER USAGE :                                                        */
/*     msg: The message to print to buffer, uses printf syntax              */
/* ------------------------------------------------------------------------ */
void print(char const *msg, ...)
{
    size_t sizeRequired = snprintf(NULL, 0, "%s: %s (%d)", msg, strerror(errno), errno);
    char  *buffer = NULL;
    char  *tempBuffer = NULL;
    int sizeModifier;

    va_list args;

    if (sizeRequired <= 0)
    {
        return;
    }
    tempBuffer = malloc(sizeof(char));
    /* Do nothing if empty */
    if ((buffer = malloc(sizeRequired + 1)) == NULL)
    {
        free(buffer);
        free(tempBuffer);
        return;
    }

    while (1)
    {
        va_start(args, msg);
        sizeModifier = vsnprintf(buffer, sizeRequired, msg, args);
        va_end(args);

        /* Check for error */

        if (sizeModifier < 0)
        {
            free(buffer);
            free(tempBuffer);
            return;
        }
        /* If that works, exit loop */
        if (sizeModifier < sizeRequired)
        {
            break;
        }
        /* Still need more space, keep expanding */
        sizeRequired = sizeModifier + 1;

        /* To be honest I don't entirely understand the inner */
        /* workings of this if statement, however it's been */
        /* tested extensively and taken as a modified example */
        /* from the man page */
        // free before overwrite
        free(tempBuffer);
        // You are causing an issue with overfreeing.
        // I will find you
        // Stop running
        if ((tempBuffer = realloc(buffer, sizeRequired)) == NULL)
        {
            free(buffer);
            free(tempBuffer);
            return;
        }
        else
        {
            buffer = tempBuffer;
        }

    }
    /* write out, and clean up */
    write(1, buffer, strlen(buffer));
    free(buffer);
    free(tempBuffer);
}
