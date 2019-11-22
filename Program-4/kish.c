#include "utils.h"
#include "signalHandlers.h"
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

char *CWD_TEMP = "/home/user";
// Defined pipes
#define STDIN  0
#define STDOUT 1
#define STDERR 2
// Magic numbers
// MAGICNUMBER: 20, seems like a sane default for your avg command
#define DEFAULT_INPUT_LENGTH 2

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
                printf("47 [%u]\n", string);
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
        write(2, "Error allocating memory for input, exiting...\n", (sizeof(char) * 48 ));
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
    savedArgs = malloc(sizeof(char) * argsLen);
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
        argArray[0] = malloc(sizeof(char) * (strlen(args)) + 1);
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

    // Set null term
//    argArray[i] = malloc(sizeof(int));
    argArray[*argAmt] = 0;

    // Copy back ref
    strcpy(args, savedArgs);

    // Free things
    free(currentArg);
    free(delimterArr);
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

void handleStandardFDOverwrites(int fdStdin, int fdStdout, int fdStderr)
{
    if (fdStdin != -1)
    {
        close(STDIN);
        dup(fdStdin);
    }
    if (fdStdout != -1)
    {
        close(STDOUT);
        dup(fdStdout);
    }
    if (fdStderr != -1)
    {
        close(STDERR);
        dup(fdStderr);
    }
}

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
    pid_t childPID;

    // Found exit command
    builtin = evaluateBuiltins(commandArgs, fdStdout);
    if (builtin == 1)
    {
        return 0;
    }

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


int parseReturnStatus(int childPID)
{
    int exitStatus;
    int returnStatus;

    waitpid(childPID, &returnStatus, 0);

    if (WIFEXITED(returnStatus))
    {
        exitStatus = WEXITSTATUS(returnStatus);
        if (exitStatus != 0)
        {
            print("[%d] %s\n", exitStatus, strerror(exitStatus));
            if (exitStatus = 2)
            {
                // Defined expected behavior
                // https://en.wikpedia.org/wiki/Exit_status
                return 127;
            }
        }
        else
        {
            // Guarenteed 0
            return exitStatus;
        }
    }
    else
    {
        // No idea what happened
        return -1;
    }
}

int runMultipleCommands(char** argsListPipeSplit, int* argsAmtPipeSplit)
{
    int i, j;
    int returnStatus;
    int existStatus;
    int builtin;
    pid_t childPID;
    int **pipes;
    char **commandArgs;
    int *argAmt;
    int fdInputRedir;
    int fdOutputRedir;

    // Make pipes
    pipes = malloc(sizeof(int) * (*argsAmtPipeSplit - 1));
    argAmt = malloc(sizeof(int));
    argAmt[0] = 0;
    for ( i = 0; i < (*argsAmtPipeSplit - 1); i++)
    {
        pipes[i] = malloc(sizeof(int) * 2);
        if (pipe(pipes[i]) != 0)
        {
            write(2, strerror(errno), strlen(strerror(errno)));
            return -1;
        }
    }
    // Parse into commands
    for ( i = 0; i < *argsAmtPipeSplit; i++)
    {
        commandArgs = splitArgs(argsListPipeSplit[i], argAmt, ' ');

        childPID = fork();

        if (childPID == -1)
        {
            write(2, "Failed to fork new process.\n",sizeof(char) * 29 );
            // TODO: print error
            return -2;
        }
        if (childPID == 0)
        {
            fdInputRedir = -1;
            fdOutputRedir = -1;
            if ( i == 0)
            {
                // Look for incoming
                // look for redirect in
                for ( j = 0; j < *argAmt; j++)
                {
                    if (strcmp(commandArgs[j], "<") == 0)
                    {
                        if (commandArgs[j+1] == NULL)
                        {
                            write(2, "Invalid redirect\n", 18);
                            return -2;
                        }
                        fdInputRedir = open(commandArgs[j+1], O_RDONLY);
                        if (fdInputRedir == -1)
                        {
                            write(2, "Invalid redirect\n", 18);
                            return -2;
                        }
                        // Overwrite value to null term before redir
                        commandArgs[j] = 0;
                    }
                }
                handleStandardFDOverwrites(fdInputRedir, pipes[i][1], -1);
            }
            else if ( i == (*argsAmtPipeSplit - 1))
            {
                // Look for outgoing
                for ( j = 0; j < *argAmt; j++)
                {
                    if (strcmp(commandArgs[j], ">") == 0)
                    {
                        if (commandArgs[j+1] == NULL)
                        {
                            write(2, "Invalid redirect\n", 18);
                            return -2;
                        }
                        fdOutputRedir = open(commandArgs[j+1], O_CREAT | O_RDWR);
                        if (fdOutputRedir == -1)
                        {
                            write(2, "Invalid redirect\n", 18);
                            return -2;
                        }
                        // Overwrite value to null term before redir
                        commandArgs[j] = 0;
                    }
                }
                handleStandardFDOverwrites(pipes[i-1][0], fdOutputRedir ,-1);
            }
            else
            {
                // Attach both to pipe
                handleStandardFDOverwrites(pipes[i-1][0], pipes[i][1] ,-1);
            }
            // There's no way to really control FDs after they move out of the
            // process space, so we assume the exec'd program will close them.
            errno = 0;
            execvp(commandArgs[0], commandArgs);
            // If we get here execvp error occured
            exit(errno);
        }
        else
        {
            // We'll keep going as we only need to wait on the final proc
            // Cleanup for next run
            for ( j = 0; j < *argAmt; j++)
            {
                free(commandArgs[j]);
            }
            free(commandArgs);
        }
    }
    // Free spares
    free(argAmt);

    for ( i = 0; i < (*argsAmtPipeSplit - 1); i++)
    {
        free(pipes[i]);
    }
    free(pipes);
    // Wait on final PID
    returnStatus = parseReturnStatus(returnStatus);

    return returnStatus;
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
    // Format: "(lastProgramReturnCode) shell_name [currentDirectory] => "
    //print("(%d) %s [%s] => ", statusCode, basename(argv[0]), CWD);
    print("(%d) %s => ", statusCode, basename(processName));

    // Get input
    /* TODO: Should really be doing this inside the function
     * and returning a pointer to it, but I've been struggling
     * hard with dynamicly allocated memory segments recently
     */
    args = malloc(sizeof(char) * DEFAULT_INPUT_LENGTH);
    args = input(args);
    printf("{%s}\n", args);
    /* Do the command stuff */
    // Eval pipes
    argsAmtPipeSplit = malloc(sizeof(int));
    argsAmtPipeSplit[0] = 0;
    argsListPipeSplit = splitArgs(args, argsAmtPipeSplit, '|');

    // strip white space from pipe split
    for ( i = 0; i < *argsAmtPipeSplit; i++)
    {
        trim(argsListPipeSplit[i]);
    }


    if (*argsAmtPipeSplit == 1)
    {
        argAmt = malloc(sizeof(int));
        // No pipe, parse args
        argsList = splitArgs(args, argAmt, ' ');
        statusCode = runCommand(argsList, -1, -1, -1);
        for ( j = 0; j < (*argAmt + 1); j++)
        {
            free(argsList[j]);
        }
        free(argsList);
        free(argAmt);
    }
    else
    {
        statusCode = runMultipleCommands(argsListPipeSplit, argsAmtPipeSplit);
    }

    // Done with arrays, clean up
    // Pipe Split
    if (*argsAmtPipeSplit != 0)
    {
        for( i = 0; i < (*argsAmtPipeSplit + 1); i++)
        {
            // Free individual args
            print("[%d] => {%s}\n", i, argsListPipeSplit[i]);
            free(argsListPipeSplit[i]);
        }
        free(argsListPipeSplit);
    }
    // Free up arg array

    free(argsAmtPipeSplit);
    free(args);
    exit(0);
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
