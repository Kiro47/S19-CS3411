#include "simpleshell.h"

#include <stddef.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

/*
 * trim(char *string)
 *      Strips leading and trailing whitespace from a string, modifies in place.
 *      Whitespace is defined as ' ', tabs, or newlines
 * args:
 *      string: Character array to strip whitespace
 */
void trim(char *string)
{
    int index, i;

    // Trim leading white spaces
    index = 0;
    while(string[index] == ' ' || string[index] == '\t' || string[index] == '\n')
    {
        index++;
    }

    // Left shift characters over whitespace
    i = 0;
    while(string[i + index] != '\0')
    {
        string[i] = string[i + index];
        i++;
    }
    // Redefine null terminator
    string[i] = '\0';

    // Trim trailing white spaces
    i = 0;
    index = -1;
    while(string[i] != '\0')
    {
        if(string[i] != ' ' && string[i] != '\t' && string[i] != '\n')
        {
            index = i;
        }

        i++;
    }

    // Reset null terminator
    string[index + 1] = '\0';
}

/* freeStringArray(char** stringArrayList, int* stringArraySize)
 *      Frees a 2d Array of void** as well as the int tracker
 *      for array int* stringArraySize
 *  Args:
 *      stringArrayList: 2d array to free
 *      stringArraySize: tracker int to free
 */
void freeStringArray(char** stringArrayList, int* stringArraySize)
{
    int i;
    for ( i = 0; i < (*stringArraySize + 1); i++)
    {
        free(stringArrayList[i]);
    }
    free(stringArrayList);
    free(stringArraySize);
}

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
    current_size = DEFAULT_INPUT_LENGTH;
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


/*
 * splitArgs(char *args, int *argAmt)
 *      Splits up args string of size argAmt into an array of strings created
 *      by splitting the args up by a delimitter (' ' a space).
 * args:
 *  args:   Command line arguments being passed in
 *  argAmt: An overwritten integer which will have the number of arguments
 *          created in it.
 * returns:
 *   char**: An array of character arrays (strings) of size argAmt created by
 *           splitting args by the delimitter.
 *           Note: All of the character arrays, as well as the array of
 *           characters arrays are allocated in memory and need to be freed
 *           when done with.
 */
char **splitArgs(char *args, int *argAmt, char *delimiter)
{
    int i;
    int argsLen;
    char *savedArgs;
    char *currentArg;
    char *placedArg;
    char **argArray;

    argsLen = (int) strlen(args);
    argAmt[0] = 0;
    // Save for unmodified return
    savedArgs = malloc(sizeof(char) * (argsLen + 1));
    strcpy(savedArgs, args);

    // Count the number of expected arg
    for (i = 0; i < argsLen; i++)
    {
        // We only ever use one delimiter, if we used multiple
        // we'd need to cycle over them here.
        if (args[i] == delimiter[0])
        {
            argAmt[0]++;
        }
    } argAmt[0]++;

    // Allocate ptr array, +1 for null term
    argArray = malloc(sizeof(char *) * ((*argAmt) + 1));

    if (*argAmt == 1)
    {
        // Only one req
        argArray[0] = malloc(sizeof(char) * (strlen(args)) + 1);
        strcpy(argArray[0], args);
    }
    else
    {
        // Reset counter
        i = 0;
        // Build delimiter
        // 1 char + null term

        currentArg = strtok(args, delimiter);
        while(currentArg != NULL)
        {
            if (i > *argAmt)
            {
                write(2, "Error splitting args\n", sizeof(char) * 22);
                break;
            }
            argArray[i] = malloc(sizeof(char) * (strlen(currentArg) + 1));
            strcpy(argArray[i++], currentArg);
            currentArg = strtok(NULL, delimiter);
        }
        free(currentArg);
    }

    // Set null term
//    argArray[i] = malloc(sizeof(int));
    argArray[*argAmt] = 0;

    // Copy back ref
    strcpy(args, savedArgs);

    // Free things
    free(savedArgs);
    return argArray;
}

