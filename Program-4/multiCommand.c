#include "shell.h"
#include "utils.h"
#include "simpleshell.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/*
 * childRunner(int* apipe, int i, int savedStdin, int saveStdout)
 *  Runs a child process for runMultipleCommands
 *
 *  Args:
 *      apipe: The pipes for children to use on pass
 *
 */
int childRunner(char ** argsListPipeSplit, int* apipe, int i)
{
    int fdOutputRedir;
    int fdInputRedir;
    int j;
    int *argAmt;
    int builtin;
    char **commandArgs;

    argAmt = malloc(sizeof(int));
    argAmt[0] = 0;
    // Doing this otherwise valgrind complains about stack allocation
    commandArgs = splitArgs(argsListPipeSplit[i], argAmt, " ");

    builtin = evaluateBuiltins(commandArgs, STDOUT);
    if (builtin == 1)
    {
        freeStringArray(commandArgs, argAmt);
        return 0;
    }

    close(apipe[1]); // close write end of pipe for children
    if ( i == (*argAmt - 1))
    {
        // Look for outgoing redir
        for ( j = 0; j < *argAmt; j++)
        {
            if (strcmp(commandArgs[j], ">") == 0)
            {
                if (commandArgs[j+1] == NULL)
                {
                    write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                    return -2;
                }
                fdOutputRedir = open(commandArgs[j+1],
                        O_CREAT|O_RDWR|O_CLOEXEC, 0644);
                if (fdOutputRedir == -1)
                {
                    write(STDERR, "Invalid redirect\n", sizeof(char) * 18);
                    write(STDERR, strerror(errno),
                            sizeof(char) * strlen(strerror(errno)));
                    return -2;
                }
                // Dup
                close(STDOUT);
                dup(fdOutputRedir);
                // Strip from command Args
                commandArgs[j] = 0;
                commandArgs[j + 1] = 0;
                break;
            }
        }
    }
    if ( i != 0)
    {
        close(STDIN);
        dup(apipe[0]);
    } // if not sort, clone pipe read end into stdin
    if ( i == 0)
    {
        // Look for outgoing redir
        for ( j = 0; j < *argAmt; j++)
        {
            if (strcmp(commandArgs[j], "<") == 0)
            {
                if (commandArgs[j+1] == NULL)
                {
                    write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                    return -2;
                }
                fdInputRedir = open(commandArgs[j+1],
                        O_RDONLY|O_CLOEXEC, 0444);
                if (fdInputRedir == -1)
                {
                    write(STDERR, "Invalid redirect\n", sizeof(char) * 18);
                    write(STDERR, strerror(errno),
                            sizeof(char) * strlen(strerror(errno)));
                    return -2;
                }
                // Dup
                close(STDIN);
                dup(fdInputRedir);
                // Strip from command Args
                commandArgs[j] = 0;
                commandArgs[j + 1] = 0;
                break;
            }
        }
    }
    close(apipe[0]); // close extra read end of pipe
    free(argAmt);
    execvp(commandArgs[0],commandArgs);
    exit(1);
}

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
int runMultipleCommands(char** argsListPipeSplit, int* argsAmtPipeSplit)
{
    int i;
    int returnStatus;
    int *apipe;
    int exitCode;
    int *code;
    int isParent;
    int childReturn;
    int lastChild;
    int saveStdout;
    saveStdout = dup(STDOUT);
    apipe = malloc(sizeof(int) * 2);
    // Reverse build command chain
    for ( i = (*argsAmtPipeSplit - 1); i >= 0; i--)
    {
        pipe(apipe);
        isParent = fork();

        if (!isParent) {
            // handle children
            childReturn =childRunner(argsListPipeSplit, apipe, i);
            if (childReturn != 0)
            {
                return childReturn;
            }
        }
        else
        {
            if(i == (*argsAmtPipeSplit - 1))
            {
                // save last child's pid
                lastChild = isParent;
            }
            // close read end of pipe for parent
            close(apipe[0]);
            // close stdout
            close(STDOUT);
            if(i!=0)
            {
                // if not sort, duplicate pipe write end into stdout
                dup(apipe[1]);
            }
            // close extra pipe end
            close(apipe[1]);
            if(i==0)
            {
                // if in shell, restore stdout to terminal
                dup2(saveStdout, STDOUT);
                waitpid(lastChild, &returnStatus,0);
            }
        }
    }

    // Valgrind got very angry unless I did it some weird way like this
    // stack/heap allocations
    code = malloc(sizeof(int));
    parseReturnStatus(returnStatus, code);
    exitCode = *code;
    free(code);

    free(apipe);
    return exitCode;
}


