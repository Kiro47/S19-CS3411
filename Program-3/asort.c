#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <libgen.h>

#define returnStat(intValue) (intValue ? "fail" : "success")
#define BINARY_PROG "/usr/bin/sort"
#define BINARY_OPTS "-o"
char *OUTPUT_DIR = "SORTED/";

typedef struct
{
    int fileName; // argc value associated with
    int pid; // PID of the process running
    int returnStatus; // Status after execution
} processData;

/*
 * helpMsg()
 *      Prints the normal usage message
 */
void helpMsg(char *progName)
{
    print("%s <file1> <file2> <file3> ... <filename>\n", progName);
}

void clearBuffer(char *buffer, int size)
{
    int i;
    for (i = 0; i <= size; i++)
    {
        buffer[i] = 0;
    }
}

/*
 * doesDirExist(char *dirName)
 *      Checks if a directory exists
 *
 * dirName: Name of directory to check for
 *
 * Returns:
 *  int:
 *      -1: Error occured
 *       0: Does not exist
 *       1: Does exist
 */
int doesDirExist(char *dirName)
{
    struct stat *stats;
    int returnVal;
    stats = malloc(sizeof(struct stat));
    stat(dirName, stats);
    if (S_ISDIR(stats->st_mode))
    {
        returnVal= 1;
    }
    else
    {
        returnVal = 0;
    }
    free(stats);
    return returnVal;
}

int makeDir(const char *dirName)
{
    if (mkdir(dirName, 0755) != 0)
    {
        switch(errno)
        {
            case EACCES:
                print("Error, cannot create directory.\n");
                break;
            case EDQUOT:
                print("Error, over allocation quota.\n");
                break;
            case EEXIST:
                // You should never get here actually.
                // Already exists, don't care?
                return 0;
            case ELOOP:
                print("Error, too many symbolic links\n.");
                break;
            case EMLINK:
                print("Error, max links exceeded.\n");
                break;
            case ENAMETOOLONG:
                print("Error, pathname too long.\n");
                break;
            case ENOENT:
                print("Error, does not exist!\n");
                break;
            case ENOSPC:
                print("Error, disk quot exhausted.\n");
                break;
            case EPERM:
                print("Error, no permissions.\n");
                break;
            case EROFS:
                print("Error, read only filesystem.\n");
                break;
            default:
                print("Error, we don't know what went wrong.\n");
                break;
        }
        return 1;
    }
    return 0;

}
/*
 * doesFileExist(char *fileName)
 *      Checks if a file exists
 *
 * fileName: Name of file to check for
 *
 * Returns:
 *  int:
 *      -1: Error occured
 *       0: Does not exist
 *       1: Does exist
 */
int doesFileExist(char *fileName)
{
    if (access(fileName, F_OK) != -1)
    {
        return 1;
    }
    return 0;
}

/*
 * checkIfAllFilesExist(int argc, char **argv)
 *      Checks if all given files exist
 *
 *  argc: Number of files in the array
 *  argv: Filenames in an array to check for
 *
 *  Returns:
 *    int:
 *      -1: Error occured
 *       0: All files exist
 *      1+: Number of files that do not exist
 */
int checkIfAllFilesExist(int argc, char **argv)
{
    int i;
    int fail;
    fail = 0;

    for (i = 0; i < argc; i++)
    {
        if (doesFileExist(argv[i]) != 1)
        {
            // file doesn't exist or errored
            print("Error file [%s] does not exist!\n", argv[i]);
            fail += 1;
        }
    }

    return fail;
}



int main(int argc, char **argv)
{
    int **childrenPIDs;
    int i, j;
    processData* pData;
    int childPID;
    int dirError;
    int returnStatus;
    int failedProcs;
    pid_t status;
    char *args[5];
    int largestStringLength;
    dirError = 0;
    returnStatus = 0;

    if ( argc <= 1)
    {
        helpMsg(argv[0]);
        return 0;
    }

    /*
     * Annoyingly enough we're not supposed to handle this.
    if (!checkIfAllFilesExist(argc, argv))
    {
        print("Error with files, bailing.\n");
        return -1;
    }
    */

    // Check if dir exists, if not make it
    if (doesDirExist(OUTPUT_DIR) != 1)
    {

        if(makeDir(OUTPUT_DIR) != 0)
        {
            return -1;
        }
    }

    largestStringLength = 0;
    for (i = 0; i < argc; i++)
    {
        if (strlen(argv[i]) > largestStringLength)
        {
            largestStringLength = strlen(argv[i]);
        }
    }

    // Set args memory allocations
    args[0] = malloc(sizeof(BINARY_PROG));
    args[1] = malloc(largestStringLength * sizeof(char));
    args[2] = malloc(sizeof(BINARY_OPTS));
    args[3] = malloc((strlen(OUTPUT_DIR) + largestStringLength + 1)
            * sizeof(char));
    args[4] = malloc(sizeof(NULL));
    // Spawn processes
    pData = malloc((argc-1) * sizeof(processData));
    returnStatus = 0;
    for (i = 1; i < argc; i++)
    {
        childPID = fork();
        if (childPID == 0)
        {
            // Close pipes because we don't care about output
            close(1);
            close(2);
            // start sort
            args[0] = BINARY_PROG;
            args[1] = argv[i];
            args[2] = BINARY_OPTS;
            strcpy(args[3], OUTPUT_DIR);
            strcat(args[3], basename(argv[i]));
            args[4] = NULL;
            execvp(BINARY_PROG,args);
            // Set return status if there's break in execvp
            pData[i-1].returnStatus = errno;
            return;
        }
        else
        {
            // Set child information
            pData[i-1].pid = childPID;
            pData[i-1].fileName = i;
        }
    }

    // Await for all children
    for (i = 0; i < (argc - 1); i++)
    {

        // Wait on child
        status = waitpid(pData[i].pid, &returnStatus, 0 );
        if (returnStatus == -1)
        {
            // error with wait
            print("%s", strerror(errno));
            pData[i].returnStatus = errno;
            continue;
        }
        if (WIFEXITED(status))
        {
            pData[i].returnStatus = WEXITSTATUS(returnStatus);
        }
        else
        {
            pData[i].returnStatus = WEXITSTATUS(returnStatus);
        }
    }


    // Did anything fail?
    failedProcs = 0;
    for (i = 0; i < (argc-1); i++)
    {
        if (pData[i].returnStatus != 0)
        {
            failedProcs += 1;
        }
    }
    // Print reports
    if (failedProcs != 0)
    {
        for (i =0; i < (argc-1); i++)
        {
            char *buffer;
            int size = sizeof(basename(argv[pData[i].fileName])) +
                sizeof(returnStat(pData[i].returnStatus)) +
                sizeof(pData[i].returnStatus) +
                9;
            buffer = malloc(size);
            sprintf(buffer, "%s : %s (%d)\n",
                    basename(argv[pData[i].fileName]),
                    returnStat(pData[i].returnStatus),
                    pData[i].returnStatus);
            buffer[size-1] = 0;
            write(1, buffer, size);
            clearBuffer(buffer, size);
            free(buffer);

        }
        print("%d out of %d files successfully sorted!\n",
                ((argc-1) - failedProcs), (argc-1));
    }
    else
    {
        print("All files successfully sorted.\n");
    }


    free(pData);
    free(args[0]);
    free(args[1]);
    free(args[2]);
    free(args[3]);
    free(args[4]);
    return 0;
}
