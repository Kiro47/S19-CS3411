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
char parseActions(char **argv);

/*
 * doesFileExist(char *filename)
 *      Checks if a file of filename exists already
 *  filename: Name/path of the file to check for
 *
 *  returns:
 *      int:
 *          -1 if an error occured opening file
 *           0 if file is readable and exists
 */
int doesFileExist(char *filename);

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
int verifyArchive(int fileDescriptor);

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
char* getFilename(int fdArchive, int nameLocation);


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

