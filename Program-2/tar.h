#ifndef PROGRAM_2_TAR_H
#define PROGRAM_2_TAR_H
/*
 *  A file containers methods which will be requied in
 *  both ctar and utar programs.
 *
 */

#define TAR_MAGIC_VAL 0x63746172

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

/*
 *  writeToFile()
 *      Write the contents of an address range (startOfFile - endOfFile)
 *      at the address point of writePoint
 *
 *  fileDescriptor: fileDescriptor to write to
 *  writeContents:  data to write to the file
 *  contentsSize:   amount of data to write from writeContents
 */
void writeToFile(int fileDescriptor, char** writeContents, int contentsSize);

#endif

