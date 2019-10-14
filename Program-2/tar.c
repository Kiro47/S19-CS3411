#include "tar.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


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

typedef struct
{
  int  magic;             /* This must have the value  0x63746172.                        */
  int  eop;               /* End of file pointer.                                         */
  int  block_count;       /* Number of entries in the block which are in-use.             */
  int  file_size[4];      /* File size in bytes for files 1..4                            */
  char deleted[4];        /* Contains binary one at position i if i-th entry was deleted. */
  int  file_name[4];      /* pointer to the name of the file.                             */
  int  next;              /* pointer to the next header block.                            */
} tarHeader;


