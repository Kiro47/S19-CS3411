#ifndef PROGRAM_4_MULTICOMMAND_H
#define PROGRAM_4_MULTICOMMAND_H

/*
 * childRunner(int* apipe, int i, int savedStdin, int saveStdout)
 *  Runs a child process for runMultipleCommands
 *
 *  Args:
 *      apipe: The pipes for children to use on pass
 *
 */
int childRunner(char ** argsListPipeSplit, int* apipe, int i);

/*
 * runMultipleCommands(char** argsListPipeSplit, int* argsAmtPipeSplit)
 *      Runs a series of commands split by pipes
 *  Args:
 *      argsListPipeSplit: A list of command strings to run, from a master command
 *                          delimitted by a '|'
 *      argsAmtPipeSplit: Length tracker of argsListPipeSplit
 *  Returns:
 *      int: Status of previous command
 */
int runMultipleCommands(char** argsListPipeSplit, int* argsAmtPipeSplit);

#endif
