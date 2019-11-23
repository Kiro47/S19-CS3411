#ifndef PROGRAM_4_SHELL_H
#define PROGRAM_4_SHELL_H

/*
 * genericHandler(int signum)
 *      Very generic signal handler function for debugging mostly.
 *      Catches a bound signal, prints it, and continues execution.
 */
void genericHandler(int signum);

/*
 * void parseReturnStatus(int returnStatus, int *code)
 *      Parses the returnStatus variable into an exit code
 *  Args:
 *      returnStatus: Return status from a wait() call
 *      code: Estimated command return code
 */
void parseReturnStatus(int returnStatus, int *code);

char* getCWD(void);

/*
 * evaluateBuiltins(char **commandArgs, int fdStdout)
 *      Evalutes shell builtin calls from commandArgs
 *  Args:
 *      commandArgs: Command argument array to parse from
 *      fdStdout: Where to echo too (if echo is selected)
 *  Returns:
 *      int:
 *          127: No dir found
 *          1: Every is fine
 */
int evaluateBuiltins(char **commandArgs, int fdStdout);

/*
 * void printShellPrompt(int statusCode, char* processName)
 *      Prints the shell prompt for user input
 * args:
 *  statusCode: Status code of previous command
 *  processName: Process name of the program
 */
void printShellPrompt(int statusCode, char* processName);

/*
 * spawnShell(char* processName, int statusCode)
 *      Spawns the interactive portion of the shell
 * args:
 *      processName: Name/Path of the process used to launch the shell
 *      statusCode: Status code of the previously executed command
 * returns:
 *   int:
 *     0: Normal execution
 *     1: `exit` builtin used, return and exit program
 */
void spawnShell(char* processName, int *statusCode);
#endif
