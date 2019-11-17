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

void getChar(int stdinFD, char* character);

#endif
