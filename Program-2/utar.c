#include "utils.h"
#include "tar.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

void printHelp(int invalid)
{
    print("To extract contents of the archive file :\n");
    print("utar <archive-file-name>\n");
}

void writeNewFile(int fdArchive, int fdNewFile, int fileSize)
{
    int bytesRead;
    int totalBytesRead;
    char byte;

    bytesRead = 0;
    totalBytesRead = 0;
    lseek(fdNewFile, 0, SEEK_SET);

    while (totalBytesRead <= fileSize)
    {
        bytesRead = read(fdArchive, &byte, sizeof(char));
        write(fdNewFile, &byte, sizeof(char));
        totalBytesRead += bytesRead;
        if (bytesRead == 0)
        {
            print("Unable to finish unpacking file, no contents left\n");
            print("Corrupted archive?\n");
            return;
        }
    }
}

/*
 * unpackArchive(char *archiveName)
 *      Unpacks a file if it is an archive
 *
 *  archiveName: Name of the arhcive to unpack
 *
 *  returns:
 *      int:
 *         -1: Archive not completely unpacked
 *          0: Success
 *          1: File is not an archive
 */
int unpackArchive(char *archiveName)
{
    int fdArchive;
    int fdUnpack;
    int i;
    char *filename;
    tarHeader *header;

    header = malloc(sizeof(*header));

    fdArchive = open(archiveName, O_RDONLY, 0444);

    // Verify if the file is an archive
    if (verifyArchive == 0)
    {
        close(fdArchive);
        free(header);
        return 1;
    }

    // Start unpacking
    lseek(fdArchive, 0, SEEK_SET);
    read(fdArchive, header, sizeof(*header));

    do
    {
        // 4 hard coded from maximum filers per header
        for (i = 0 ; i < 4; i++)
        {
            if (header->deleted[i] == 0 && header->file_name[i] != 0)
            {
                // File exists and *should* be unpacked
                filename = getFilename(fdArchive, header->file_name[i]);
                // Tests if we should write
                if (doesFileExist(filename) == 0)
                {
                    print("File already exists!  Stopping unpack.\n");
                    free(header);
                    close(fdArchive);
                    return -1;
                }
                // Unpack file
                fdUnpack = open(filename, O_RDWR|O_CREAT, 0644);
                // NOTE: assumes no other fdArchive actions have taken place
                lseek(fdArchive, strlen(filename) + 1, SEEK_CUR);
                writeNewFile(fdArchive, fdUnpack, header->file_size[i]);
                close(fdUnpack);
            }
        }
    } while(header->next != 0);
    close(fdArchive);
    free(header);
    return 0;
}

void listFiles()
{

}

/*
 * main(int argc, char **argv)
 *      Program to unpack the archive
 *
 *  Returns:
 *      int:
 *          -1: Error opening file
 *           0: Success
 *           1: File does not exist
 *           2: File is not an archive
 */
int main(int argc, char **argv)
{
    int existStatus;
    if (argc != 2)
    {
        printHelp(1);
    }
    else
    {
        existStatus = doesFileExist(argv[1]);
        if (existStatus == -1)
        {
            print("Error occured opening file [%s]\n", argv[1]);
            return -1;
        }
        else if (existStatus == 1)
        {
            print("File [%s] does not exist!\n", argv[1]);
            return 1;
        }
        else
        {
            print("Unpacking archive: [%s]\n", argv[1]);
            if (unpackArchive(argv[1]) == 1)
            {
                return 2;
            }
        }
    }
    return 0;
}
