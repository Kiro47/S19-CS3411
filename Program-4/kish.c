#include "utils.h"
#include "signalHandlers.h"

#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>

char *CWD_TEMP = "/home/user";

/*
 * input()
 *      Gathers user input from stdin after a use enters a linefeed.
 *
 * returns:
 *   char**:  The string that the user has entered
 */
char* input()
{
    char *string;
    char curChar;
    int current_size;
    string = malloc(10);
    current_size = 0;

    if (string != NULL)
    {
        char c = 0;
        unsigned int i = 0;
        while (read(0, &curChar, sizeof(char)) != 0)
        {
            if (curChar == '\n')
            {
                if (i == current_size)
                {
                    current_size = i + 10;
                    string = realloc(string, current_size);
                }
                break;
            }
            string[i++] = curChar;
        }
        string[i] = 0;
    }
    /* TODO: REMOVE DEBUG: */
    /*
    printf("inp: [%s]\n", string);
    */
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
char **splitArgs(char *args, int *argAmt, char delimiter)
{
    int i;
    int argsLen = (int) strlen(args);
    char *savedArgs;
    char *currentArg;
    char *placedArg;
    char *delimterArr;
    char **argArray;

    argAmt[0] = 0;
    // Save for unmodified return
    savedArgs = malloc(argsLen);
    strcpy(savedArgs, args);

    // Count the number of expected arg
    for (i = 0; i < argsLen; i++)
    {
        if (args[i] == delimiter)
        {
            argAmt[0]++;
        }
    } argAmt[0]++;

    // Allocate ptr array, +1 for null term
    argArray = malloc(sizeof(char *) * ((*argAmt) + 1));

    if (*argAmt == 1)
    {
        // Only one req
        argArray[0] = malloc(sizeof(char) * strlen(args));
        strcpy(argArray[0], args);
    }
    else
    {
        // Reset counter
        i = 0;
        // Build delimiter
        // 1 char + null term
        delimterArr = malloc(2 * sizeof(char));
        delimterArr[0] = delimiter;
        delimterArr[1] = 0;

        currentArg = strtok(args, delimterArr);
        while(currentArg != NULL)
        {
            if (i > *argAmt)
            {
                print("Error splitting args, {i = %d} {argAmt = %d}\n", i, *argAmt);
                break;
            }
            argArray[i] = malloc(sizeof(char) * strlen(currentArg));
            strcpy(argArray[i++], currentArg);
            currentArg = strtok(NULL, delimterArr);
        }
    }

    free(currentArg);
    free(delimterArr);
    // Set null term
    argArray[*argAmt] = 0;
    /* TODO: REMOVE DEBUG: */
    /*
    for (i = 0; i < (*argAmt); i++)
    {
        printf("arg[%d] = %s\n", i, argArray[i]);
    }
    */
    strcpy(args, savedArgs);
    free(savedArgs);
    return argArray;
}

/*
 * genericHandler(int signum)
 *      Very generic signal handler function for debugging mostly.
 *      Catches a bound signal, prints it, and continues execution.
 */
static void genericHandler(int signum)
{
    print("Sign: [%d]\n", signum);
    return;
}

/*
 * runCommand(char** commandArgs)
 *      Forks a new process which runs the passed in command
 *  args:
 *      commandArgs: An array of args used to run the command.
 *                   0 is executed to be the executable, and
 *                   the array is expected to be null terminated.
 *  returns:
 *      int: Exit code of ran command
 *
 */
int runCommand(char** commandArgs)
{
    int returnStatus;
    int exitStatus;
    pid_t childPID;

    childPID = fork();
    signal(SIGTTIN, genericHandler);
    if (childPID == -1)
    {
        print("Failed to fork new process.\n");
        print("Error: [%s]\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
    if (childPID == 0)
    {
        // Child thread
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
        if (WIFEXITED(returnStatus))
        {
            exitStatus = WEXITSTATUS(returnStatus);
            if (exitStatus != 0)
            {
                print("[%d] %s\n", exitStatus,strerror(exitStatus));
                if (exitStatus = 2)
                {
                    // Defined expected behavior
                    // https://en.wikipedia.org/wiki/Exit_status
                    return 127;
                }

                return exitStatus;
            }
            else
            {
                // Guarenteed 0
                return exitStatus;
            }
        }
    }
}

void stripWhiteSpace(char *string)
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

    /* Mark the next character to last non white space character as NULL */
    string[index + 1] = '\0';
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
    char **argsList;
    char **argsListPipeSplit;
    char *buffer;
    int i;
    int tempPipes[2];

    // Prompt shell
    // Format: "(lastProgramReturnCode) shell_name [currentDirectory] => "
    //print("(%d) %s [%s] => ", statusCode, basename(argv[0]), CWD);
    print("(%d) %s => ", statusCode, basename(processName));

    // Get input
    /* TODO: Should really be doing this inside the function
     * and returning a pointer to it, but I've been struggling
     * hard with dynamicly allocated memory segments recently
     */
    char *args = input();

    /* Do the command stuff */
    // Eval pipes
    argsAmtPipeSplit = malloc(sizeof(int));
    argsAmtPipeSplit[0] = 0;
    argsListPipeSplit = splitArgs(args, argsAmtPipeSplit, '|');

    // strip white space from pipe split
    for ( i = 0; i < *argsAmtPipeSplit; i++)
    {
        stripWhiteSpace(argsListPipeSplit[i]);
    }

    argAmt = malloc(sizeof(int));

    if (*argsAmtPipeSplit == 1)
    {
        // No pipe, parse args
        argsList = splitArgs(args, argAmt, ' ');
        statusCode = runCommand(argsList);
    }
    else
    {
        // Parse args
        for ( i = 0; i < *argsAmtPipeSplit; i++)
        {
            runCommand(splitArgs(argsListPipeSplit[i], argAmt, ' '));
        }
        //
        argsList = splitArgs(args, argAmt, ' ');

    }
    // TODO: Evaluate builtins

    // Done with arrays, clean up
    // Cmd Arg Clean
//    free(buffer);
    for( i = 0; i < *argAmt; i++)
    {
        // Free individual args
        free(argsList[i]);
    }
    // Free up arg array
    free(argsList);
    free(argAmt);
    // Pipe Split
    for( i = 0; i < *argsAmtPipeSplit; i++)
    {
        // Free individual args
        print("[%d] => {%s}\n", i, argsListPipeSplit[i]);
        free(argsListPipeSplit[i]);
    }
    // Free up arg array
    free(argsListPipeSplit);
    free(argsAmtPipeSplit);
    free(args);
    return statusCode;
}


int main(int argc, char **argv)
{
    char *CWD;
    int shellCode;

//    CWD = malloc(sizeof(CWD_TEMP));
//    strcpy(CWD, CWD_TEMP);

    /*
    * Do the shell stuff
    */
    while(1)
    {
        shellCode = spawnShell(argv[0], shellCode);
        if (shellCode == 1)
        {
            // Shell returned `exit`
            return 0;
        }
    }
}
