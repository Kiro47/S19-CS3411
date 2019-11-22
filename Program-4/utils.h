#ifndef PROGRAM_2_UTILS_H
#define PROGRAM_2_UTILS_H

/*
 * print():
 *   Prints the given message to stdout and dynamically allocates memory for it
 * Args:
 *   msg: The message to print to the buffer
 *   ...: Any specified args to pass into the string msg
 *        follows the same syntax as printf.
 */
void print(char const *msg, ...);

/* freeStringArray(char** stringArrayList, int* stringArraySize)
 *      Frees a 2d Array of void** as well as the int tracker
 *      for array int* stringArraySize
 *  Args:
 *      stringArrayList: 2d array to free
 *      stringArraySize: tracker int to free
 */
void freeStringArray(char** stringArrayList, int* stringArraySize);

/*
 * trim(char *string)
 *      Strips leading and trailing whitespace from a string, modifies in place.
 *      Whitespace is defined as ' ', tabs, or newlines
 * args:
 *      string: Character array to strip whitespace
 */
void trim(char *string);

/*
 * input()
 *      Gathers user input from stdin after a use enters a linefeed.
 *
 * returns:
 *   char**:  The string that the user has entered
 */
char* input(char* string);

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
char **splitArgs(char *args, int *argAmt, char *delimiter);

#endif
