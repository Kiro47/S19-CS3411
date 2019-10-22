#include "tar.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

/*
 * parseActions(char **argv)
 *      Returns single char flag for action type.
 *      assumes argv is at least of length 2.
 *      assumes flag format of '-[:alpha:]'
 *  argv: CLI args passed into program executable, args assumed to
 *        be at position 1 in array.
 *
 *  returns:
 *      char: action flag of cli args
 */
char parseActions(char **argv)
{
    // We're only using single var flags
    if (strlen(argv[1]) != 2)
    {
        return 0;
    }

    // Check for flag delimiter
    if (argv[1][0] != '-')
    {
        return 0;
    }

    // Get action flag
    if (isalpha(argv[1][1]))
    {
        return argv[1][1];
    }
    // No flag still returning nothing
    return 0;
}

/*
 * getFilename(int fdArchive, int nameLocation)
 *      Gets a filename from an archive and returns it
 *      rememeber to free me!
 *
 *  fdArchive:      fileDescriptor of archive
 *  nameLocation:   Position where filename starts (from header)
 *
 *  returns:
 *      char*: filename string
 */
char* getFilename(int fdArchive, int nameLocation)
{
    // logSize = filled
    // phySize = allocated
    int logSize;
    int phySize;
    // Return string
    char *string;
    // Copied byte
    char byte;

    logSize = 0;
    phySize = 1;

    string = (char *)malloc(sizeof(char));

    //get a char from user, first time outside the loop
    pread(fdArchive, &byte, sizeof(char), nameLocation++);

    //define the condition to stop receiving data
    while(byte != 0x0)
    {
        if(logSize == phySize)
        {
            phySize *= 2;
            string = (char *)realloc(string, sizeof(char) * phySize);
        }
        string[logSize++] = byte;
        pread(fdArchive, &byte, sizeof(char), nameLocation++);
    }
    //here we diminish string to actual logical size, plus one for \0
    string = (char *)realloc(string, sizeof(char *) * (logSize + 1));
    string[logSize] = '\0';
    free(string);
    return string;
}

/*
 * doesFileExist(char *filename)
 *      Checks if a file of filename exists already
 *  filename: Name/path of the file to check for
 *
 *  returns:
 *      int:
 *          -1 if an error occured opening file
 *           0 if file is readable and exists
 *           1 if file does not exist
 */
int doesFileExist(char *filename)
{
    int fileDescriptor;
    int returnValue;

    fileDescriptor = open(filename, O_RDONLY, 0444);

    // TODO: strip
    if (errno != 0)
    {
    }
    if (errno == 2)
    {
        // No file exists
        returnValue = 1;
    }
    else
    {
        if (fileDescriptor < 0)
        {
            print("Error opening file [%s], err: [%d]\n", filename, errno);
            print(fileDescriptor);
            returnValue = errno;
        }
        else
        {
            returnValue = 0;
        }
    }

    // Close file
    close(fileDescriptor);
    return returnValue;
}

/*
 *  verifyArchive(int fileDescriptor)
 *      Verify that a file archive has a valid header and is not a random file
 *
 *  fileDescriptor: File descriptor to look at
 *
 *  returns:
 *      int:
 *          0 if invalid
 *          1 if valid
 */
int verifyArchive(int fileDescriptor)
{
    tarHeader *header;
    off_t offset;
    int returnValue;

    header = malloc(sizeof(*header));

    offset = lseek(fileDescriptor, 0, SEEK_SET);
    // read in header
    read(fileDescriptor, header, sizeof(*header));

    returnValue = ((header->magic == TAR_MAGIC_VAL) ? 1 : 0);
    free(header);
    return returnValue;
}

/*
 *  copyToArchive(int fdSrc, int fdArchive, int startLocation)
 *          Copies a file at fdSrc to the archive file at startLocation
 *
 *  fdSrc:          fileDescriptor of file to copy from
 *  fdArchive:      fileDescriptor of archive file to write to
 *  startLocation:  Location to start writing at in the archive
 *
 *  Returns:
 *      int:    Number of bytes copied
 */
int copyToArchive(int fdSrc, int fdArchive, int startLocation)
{
    int bytesRead;
    char byteValue;
    int totalBytesRead;

    // Inits
    bytesRead = 0;
    byteValue = 0;
    totalBytesRead = 0;

    // Get to point
    lseek(fdArchive, startLocation, SEEK_SET);
    lseek(fdSrc, 0, SEEK_SET);

    // Copy file
    do
    {
        bytesRead = read(fdSrc, &byteValue, sizeof(char));
        write(fdArchive, &byteValue, sizeof(char));
        totalBytesRead += bytesRead;
    } while (bytesRead > 0);

    return totalBytesRead;
}

/*
 *  writeToFile()
 *      Write the contents of an address range (startOfFile - endOfFile)
 *      at the address point of writePoint
 *
 *  fileDescriptor: file descriptor of file to write to
 *  writeContents:  data to write to the file
 *  contentsSize:   amount of data to write from writeContents
 */
void writeToFile(int fileDescriptor, char** writeContents, int contentsSize)
{

}

/*
 * findNextHeader()
 *      Searches a file for the next available tar header and sets the
 *      currency indicator to the very beggining of that header.
 *
 *      This assumes the currencyPointer is already at a header
 *  fileDescriptor: file descriptor of file to read from
 *
 *  returns:
 *      int:
 *          -2: No header found
 *          -1: Not at header
 *          0+: Location of header
 */
int findNextHeader(int fileDescriptor)
{
    // Note currency indicators are shared across processes
    // This assumes currency counter is at a byte offset

    tarHeader *header;

        // Read in header
        read(fileDescriptor, header, sizeof(*header));

        // Verify
        if (header->magic != TAR_MAGIC_VAL)
        {
            // No header
            return -1;
        }
        else
        {
            // Header is valid
            if (header->next == 0)
            {
                // At latest header
            }
            else
            {
                // To next header
                lseek(fileDescriptor, header->next, SEEK_SET);
                read(fileDescriptor, header, sizeof(*header));
            }
        }
    return lseek(fileDescriptor, (sizeof(*header) * -1), SEEK_CUR);
}

//typedef struct
//{
//  int  magic;             /* This must have the value  0x63746172.                        */
//  int  eop;               /* End of file pointer.                                         */
//  int  block_count;       /* Number of entries in the block which are in-use.             */
//  int  file_size[4];      /* File size in bytes for files 1..4                            */
//  char deleted[4];        /* Contains binary one at position i if i-th entry was deleted. */
//  int  file_name[4];      /* pointer to the name of the file.                             */
//  int  next;              /* pointer to the next header block.                            */
//} tarHeader;
