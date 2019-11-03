#include "utils.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

char *OUTPUT_DIR = "SORTED";

typedef struct
{
    int fileName; // Pointer to name of the file
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
    struct stat stats;
    stat(dirName, &stats);
    if (S_ISDIR(stats.st_mode))
    {
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
    processData **pData;
    int i, j;
    int childPID;
    int dirError;
    int returnStatus;
    dirError = 0;
    returnStatus = 0;

    if ( argc <= 1)
    {
        helpMsg(argv[0]);
        return 0;
    }

    if (checkIfAllFilesExist(argc, argv))
    {
        print("Error with files, bailing.\n");
        return -1;
    }
    // Check if dir exists, if not make it
    if (doesDirExist(OUTPUT_DIR) != 1)
    {
        dirError = mkdir(OUTPUT_DIR, 0755);
        if (dirError == -1)
        {
            print(strerror(errno));
            print("Error creating directory [%s]\n", OUTPUT_DIR);
            return -1;
        }
    }

    // Spawn processes
    pData = malloc(sizeof (processData) * (argc - 1));
    returnStatus = 0;
    for (i = 1; i < argc; i++)
    {
        childPID = fork();
        if (childPID == 0)
        {
            // start sort

            // Set return status
            pData[i-1]->returnStatus = returnStatus;
        }
        else
        {
            // Set child information
            pData[i-1]->pid = childPID;
            pData[i-1]->fileName = *argv[i];
        }
    }

    // Await for all children
    for (i = 0; i < (argc - 1); i++)
    {
        // Wait on any child
        wait(NULL);
    }

    // Print report




    // We start at 1 because of program name offset
/*
 * old code, decided to it like on line 162 now
    while(i != 1)
    {
        for (j = 0; j < (argc - 1); i++)
        {
            // Check if active
            if (childrenPIDs[j] != 0)
            {
                childPID = childrenPIDs[j];
                waitpid(childPID, &returnStatus);
            }
        }
    }
*/
    free(childrenPIDs);
    print("test\n");
    return 0;
}
