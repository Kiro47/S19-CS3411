#include "tar.h"

#include <ctype.h>
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

    fileDescriptor = open(filename, O_RDONLY);

    print("error: %d\n",errno);
    print("fd: %d\n",fileDescriptor);
    print("error: %d\n", errno);
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

    header = malloc(sizeof(*header));

    offset = lseek(fileDescriptor, 0, SEEK_SET);
    // read in header
    read(fileDescriptor, header, sizeof(*header));

    int magic;
    magic = header->magic;

    if (header->magic == TAR_MAGIC_VAL)
    {
        return 1;
    }
    else
    {
        return 0;
    }

    return (header->magic == TAR_MAGIC_VAL) ? 1 : 0;
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
 *  fileDescriptor: file descriptor of file to read from
 */
void findNextHeader(int fileDescriptor)
{
    // Note currency indicators are shared across processes
    char currentByte;
    int byteCount;

    while (byteCount != 0)
    {
        // Read in byte by byte
        read(fileDescriptor, &currentByte, sizeof(char));
        // If it matches the magic value
        // might need to handle this closer to bit by bit than byte by byte?
        if (currentByte == TAR_MAGIC_VAL)
        {
            // read back by magic size
        }
    }
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
