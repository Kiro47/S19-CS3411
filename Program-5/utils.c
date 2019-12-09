#include "utils.h"
#include "protocol.h"

#include <stdio.h>
#include <stdlib.h>

/*
 * input()
 *      Gathers user input from stdin after a use enters a linefeed.
 *
 * returns:
 *   char**:  The string that the user has entered
 */
char* input(char* string)
{
   // char *string;
    char curChar;
    int current_size;

    //string = malloc(sizeof(char) * DEFAULT_INPUT_LENGTH);
    current_size = MAX_USERNAME_SIZE;
    if (string != NULL)
    {
        char c = 0;
        unsigned int i = 0;
        while (read(STDIN, &curChar, sizeof(char)) != 0)
        {
            if (i == current_size)
            {
                current_size = current_size + sizeof(char);
                string = realloc(string, current_size);
            }
            if (curChar == '\n')
            {

                break;
            }
            string[i++] = curChar;
        }
        string[i] = 0;
    }
    else
    {
        write(STDERR, "Error allocating memory for input, exiting...\n", (sizeof(char) * 48 ));
        exit(3);
    }
    return string;
}

