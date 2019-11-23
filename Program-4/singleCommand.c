#include "shell.h"
#include "simpleshell.h"
#include "utils.h"

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <fcntl.h>

// PRIVATE FUNC
/*
 * handleStandardFDOverwrites(int fdStdin, int fdStdout, int fdStderr)
 *      Passes in inputs to parse and set FD for STDIN, STDOUT, and STERR
 *  Args:
 *      fdStdin:    -1 for default or an FD to dupe from
 *      fdStdout: -1 for default or an FD to dupe from
 *      fdStderr: -1 for default or an FD to dupe from
 */
void handleStandardFDOverwrites(int fdStdin, int fdStdout, int fdStderr)
{
    if (fdStdin != -1)
    {
        close(STDIN);
        dup(fdStdin);
//        close(fdStdin);
    }
    if (fdStdout != -1)
    {
        close(STDOUT);
        dup(fdStdout);
//        close(fdStdout);
    }
    if (fdStderr != -1)
    {
        close(STDERR);
        dup(fdStderr);
//        close(fdStderr);
    }
}


void parseRedirects(char** commandArgs)
{
    int i;
    int j;
    int args;
    int fdOutputRedir;
    int fdInputRedir;
    fdOutputRedir = STDOUT;
    fdInputRedir = STDIN;

    args = 0;
    while (commandArgs[args] != NULL)
    {
        args++;
    }

    // Redir as the first part of a command is invalid by spec
    for (i = 1; i < args; i++)
    {
        if (strcmp(commandArgs[i], ">") == 0)
        {
            if (commandArgs[i+1] == NULL)
            {
                write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                return;
            }
            fdOutputRedir = open(commandArgs[i+1],
                    O_CREAT|O_RDWR|O_CLOEXEC, 0644);
            if (fdOutputRedir == -1)
            {
                write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                write(STDERR, strerror(errno),
                        sizeof(char) * strlen(strerror(errno)));
                return;
            }
            close(STDOUT);
            dup(fdOutputRedir);
            // Strip from command Args
            commandArgs[i] = 0;
            commandArgs[i+1] = 0;
            // Skip over next arg, we already know it
            i++;
        }
        if (strcmp(commandArgs[i], "<") == 0)
        {
            if (commandArgs[i+1] == NULL)
            {
                write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                return;
            }
            fdInputRedir = open(commandArgs[i+1],
                    O_RDONLY|O_CLOEXEC, 0444);
            if (fdInputRedir == -1)
            {
                write(STDERR, "Invalid Redirect\n", sizeof(char) * 18);
                write(STDERR, strerror(errno),
                        sizeof(char) * strlen(strerror(errno)));
                return;
            }
            close(STDIN);
            dup(fdInputRedir);
            // Strip from command Args
            for (j = i; j < args; j++)
            {
                commandArgs[j] = 0;
                strcpy(commandArgs[j], commandArgs[j+1]);
            }
            // Decrease back down one, and lower args by the two we removed
            args -= 2;
            i--;
        }
    }
}

/*
 * runCommand(char** commandArgs)
 *      Forks a new process which runs the passed in command
 *  args:
 *      commandArgs: An array of args used to run the command.
 *                   0 is executed to be the executable, and
 *                   the array is expected to be null terminated.
 *      fdStdin:    FD to set standard input as, set as -1 to not change
 *      fdStdout:   FD to set standard ouput as, set as -1 to not change
 *      fdStderr:   FD to set standard error as, set as -1 to not change
 *  returns:
 *      int: Exit code of ran command
 *
 */
int runCommand(char** commandArgs, int fdStdin, int fdStdout, int fdStderr)
{
    int returnStatus;
    int exitStatus;
    int builtin;
    int *code;
    int returnCode;
    pid_t childPID;

    // Found exit command
    builtin = evaluateBuiltins(commandArgs, fdStdout);
    if (builtin == 2)
    {
        // exit call
        return INT_MIN;
    }
    if (builtin == 1)
    {
        return 0;
    }

    childPID = fork();
    signal(SIGTTIN, genericHandler);
    if (childPID == -1)
    {
        write(STDERR, "Failed to fork new process.\n", sizeof(char) * 29);
        write(STDERR, strerror(errno), strlen(strerror(errno)));
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {
        parseRedirects(commandArgs);
        // Child thread
        // Handle file descriptor specs
        handleStandardFDOverwrites(fdStdin, fdStdout, fdStderr);
        errno = 0;
        execvp(commandArgs[0], commandArgs);
        // If we get here execvp has errored out
        exit(errno);
    }
    else
    {
        exitStatus = 0;
        // Parent thread
        wait(&returnStatus);
        code = malloc(sizeof(int));
        parseReturnStatus(returnStatus, code);
        returnCode = *code;
        free(code);
        return returnCode;
    }
}


