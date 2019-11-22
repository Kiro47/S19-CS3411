#include "shell.h"
#include "utils.h"
#include "singleCommand.h"
#include "multiCommand.h"
#include "simpleshell.h"

#include <unistd.h>
#include <libgen.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>

// sprintf
#include <stdio.h>

/*
 * genericHandler(int signum)
 *      Very generic signal handler function for debugging mostly.
 *      Catches a bound signal, prints it, and continues execution.
 */
void genericHandler(int signum)
{
    print("Sign: [%d]\n", signum);
    return;
}

/*
 * void parseReturnStatus(int returnStatus, int *code)
 *      Parses the returnStatus variable into an exit code
 *  Args:
 *      returnStatus: Return status from a wait() call
 *      code: Estimated command return code
 */
void parseReturnStatus(int returnStatus, int *code)
{
    int exitStatus;


    if (WIFEXITED(returnStatus))
    {
        exitStatus = WEXITSTATUS(returnStatus);
        if (exitStatus != 0)
        {
            write(STDERR, strerror(exitStatus), strlen(strerror(exitStatus)));
            write(STDERR, "\n", sizeof(char));
            if (exitStatus = 2)
            {
                // Defined expected behavior
                // https://en.wikpedia.org/wiki/Exit_status
                code[0] = 127;
            }
            code[0] = exitStatus;
        }
        else
        {
            // Guarenteed 0
            code[0] = exitStatus;
        }
    }
    else
    {
        // No idea what happened
        code[0] = -1;
    }
}


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
int evaluateBuiltins(char **commandArgs, int fdStdout)
{
    int i;
    /*
     * If else chains for builtin evals because doing a switch
     * would require me to setup an enum interface, which not today.
     */
    if (fdStdout == -1)
    {
        fdStdout = 1;
    }
    if (strcmp(commandArgs[0], "exit") == 0)
    {
        // Bail quick
        exit(0);
    }
    else if (strcmp(commandArgs[0], "cd") == 0)
    {
        // TODO: ... cd
        if (commandArgs[1] != NULL)
        {
            if (chdir(commandArgs[1]) == -1)
            {
                write(STDERR, strerror(errno), strlen(strerror(errno)));
                write(STDERR, "\n", sizeof(char));
                return 127;
            }
            return 1;
        }
        else
        {
            if (chdir(getenv("HOME")) == -1)
            {
                write(STDERR, strerror(errno), strlen(strerror(errno)));
                write(STDERR, "\n", sizeof(char));
                return 127;
            }
            return 1;
        }
    }
    else if (strcmp(commandArgs[0], "echo") == 0)
    {
        for (i = 1; commandArgs[i] != NULL; i++)
        {
            if (commandArgs[i] == NULL)
            {
                break;
            }
            write(fdStdout, commandArgs[i], strlen(commandArgs[i]));
            write(fdStdout, " ", sizeof(char));
        }
        write(fdStdout, "\n", sizeof(char));
        return 1;
    }
}

char* getCWD(void)
{
    int size;
    char* cwd;
    size = DEFAULT_INPUT_LENGTH;
    cwd = malloc(sizeof(char) * DEFAULT_INPUT_LENGTH);

    while (getcwd(cwd, size) == NULL)
    {
        size += DEFAULT_INPUT_LENGTH;
        cwd = realloc(cwd, size);
    }
}

/*
 * void printShellPrompt(int statusCode, char* processName)
 *      Prints the shell prompt for user input
 * args:
 *  statusCode: Status code of previous command
 *  processName: Process name of the program
 */
void printShellPrompt(int statusCode, char* processName)
{
    char *cwd;
    char* buffer;
    int allocation;
    int i;

    cwd = getCWD();
    allocation = (strlen(cwd) + strlen(basename(processName))
                + strlen(SHELL_FORMAT) + 1);

    buffer = malloc((sizeof(char) * allocation) + sizeof(int));
    for (i = 0; i < (allocation + 4); i++)
    {
        buffer[i] = 0;
    }
    sprintf(buffer, SHELL_FORMAT, statusCode, basename(processName), cwd);
    buffer[allocation-1] = 0;
    write(STDOUT, buffer, (sizeof(char) * allocation) + sizeof(int));
    free(cwd);
    free(buffer);
}

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
int spawnShell(char* processName, int statusCode)
{
    int argsLen;
    int *argAmt;
    int *argsAmtPipeSplit;
    int **pipes;
    char **argsList;
    char **argsListPipeSplit;
    char *buffer;
    char *args;
    int i, j;

    // Prompt shell
    printShellPrompt(statusCode, processName);


    args = malloc(sizeof(char) * DEFAULT_INPUT_LENGTH);
    args = input(args);
    printf("{%s}\n", args);

    /* Do the command stuff */
    // Eval pipes
    argsAmtPipeSplit = malloc(sizeof(int));
    argsAmtPipeSplit[0] = 0;
    argsListPipeSplit = splitArgs(args, argsAmtPipeSplit, "|");

    // strip white space from pipe split
    for ( i = 0; i < *argsAmtPipeSplit; i++)
    {
        trim(argsListPipeSplit[i]);
    }


    if (*argsAmtPipeSplit == 1)
    {
        argAmt = malloc(sizeof(int));
        // No pipe, parse args
        argsList = splitArgs(args, argAmt, " ");
        statusCode = runCommand(argsList, -1, -1, -1);
        freeStringArray(argsList, argAmt);
    }
    else
    {
        statusCode = runMultipleCommands(argsListPipeSplit, argsAmtPipeSplit);
    }

    // Done with arrays, clean up
    // Pipe Split
    (*argsAmtPipeSplit != 0) ?
        freeStringArray(argsListPipeSplit, argsAmtPipeSplit)
        : free(argsAmtPipeSplit);

    // Free up arg array
    free(args);
//    exit(0);
    return statusCode;
}

