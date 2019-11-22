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
#include <unistd.h>

char *CWD_TEMP = "/home/user";
// Defined pipes
#define STDIN  0
#define STDOUT 1
#define STDERR 2
// Magic numbers
// MAGICNUMBER: 20, seems like a sane default for your avg command
#define DEFAULT_INPUT_LENGTH 2
#define SHELL_FORMAT "[%d] %s (%s) => "

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
                print("Error splitting args, {i = %d} {argAmt = %d}\n", i, *argAmt);
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
                write(STDERR, strerror(exitStatus), strlen(strerror(exitStatus)));
                write(STDERR, "\n", sizeof(char));
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


void parseReturnStatus(pid_t childPID, int *code)
{
    int exitStatus;
    int returnStatus;

    waitpid(childPID, &returnStatus, 0);

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

int runMultipleCommands(char** argsListPipeSplit, int* argsAmtPipeSplit)
{
    int i, j;
    int *returnStatus;
    int existStatus;
    int builtin;
    int childPID;
    int *apipe;
    char **commandArgs;
    int *argAmt;
    int fdInputRedir;
    int fdOutputRedir;
    int exitCode;
    int isParent;

    int fd;
    int lastChild;
    // Make childPID array
//    childPID = malloc(sizeof(int));
    apipe = malloc(sizeof(int) * 2);
    argAmt = malloc(sizeof(int));
    int saveStdout;
    saveStdout = dup(STDOUT);
    // Parse into commands
    for ( i = (*argsAmtPipeSplit - 1); i >= 0; i--)
    {
        argAmt[0] = 0;

        // Doing this otherwise valgrind complains about stack allocation
        commandArgs = splitArgs(argsListPipeSplit[i], argAmt, " ");

        pipe(apipe);
        isParent = fork();

        if (!isParent) {  // execute the sub-programs
            close(apipe[1]); // close write end of pipe for children
                close(0); // close stdin for all 3 processes
            if(i != 0) {
                dup(apipe[0]);
            } // if not sort, clone pipe read end into stdin
            if(i == 0) {
                fd = open("filecomm1.txt", O_RDONLY);
                dup(fd); // clone fd of filecomm1.c to stdin for sort
            }
            close(apipe[0]); // close extra read end of pipe

            execvp(commandArgs[0],commandArgs);
            exit(1);
        }
        else {
            if(i == (*argsAmtPipeSplit - 1)) lastChild = isParent; // save last child's pid
            close(apipe[0]); // close read end of pipe for parent
            close(1); // close stdout
            if(i!=0) { dup(apipe[1]); } // if not sort, duplicate pipe write end into stdout
            close(apipe[1]); // close extra pipe end
            if(i==0){
                dup2(saveStdout, 1); // if in shell, restore stdout to terminal
                waitpid(lastChild,NULL,0);
            }
        }

    }
    // Free spares
    free(argAmt);
    printf("Parent ID: [%d]\n", getpid());
    // Wait on PIDs
    returnStatus = malloc(sizeof(int));
    // Only get final

//    exitCode = *returnStatus;
    exitCode = 0;
    free(returnStatus);

    free(apipe);
    return exitCode;
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
