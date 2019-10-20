#include "utils.h"
#include "tar.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

void print_help(int invalid)
{
    if (invalid)
    {
        print("Invalid Syntax! \nPlease refer to the following help...\n");
    }
	print("To create an empty archive file:\n");
	print("ctar -a <archive-file-name>\n");
	print("To append files to a given archive (create if not present) :\n");
	print("ctar -a <archive-file-name> file1 file2 ... filen\n");
	print("To delete a file from the archive :\n");
	print("ctar -d <archive-file-name> <file-to-be-deleted>\n");
	print("To extract contents of the archive file :\n");
	print("utar <archive-file-name>\n");
}

/*
 * createArchive(char* filename)
 *      Creates an empty archive if one does not
 *      already exist
 *  filename: Name of the file to create
 *
 *  returns:
 *      int:  1+ if created and ready, equal to file descriptor
 *      int:  0  File is not a valid archive
 *      int: -1- error code from opening.
 */
int createArchive(char* filename)
{
    int existStatus;
    int fileDescriptor;
    off_t offset;
    tarHeader *header;

    existStatus=  doesFileExist(filename);
    if (!(existStatus))
    {
        return existStatus;
    }
    fileDescriptor = open(filename, O_RDWR|O_CREAT, 0644);
    // file descriptor shouldn't have the option of being neg due to above

    // TODO: get file currency pointer

    // if currency pointer is at 0
    // dump in header
    // else return since archive already exists
    offset = lseek(fileDescriptor, 0, SEEK_END);
    if (offset == 0)
    {
        // file is empty, construct header
        header = malloc(sizeof(header));
        header->magic           = TAR_MAGIC_VAL;
        header->eop             = offset;
        header->block_count     = 0;
        header->file_size[0]    = 0;
        header->file_size[1]    = 0;
        header->file_size[2]    = 0;
        header->file_size[3]    = 0;
        header->deleted[0]      = 0;
        header->deleted[1]      = 0;
        header->deleted[2]      = 0;
        header->deleted[3]      = 0;
        header->file_name[0]    = 0;
        header->file_name[1]    = 0;
        header->file_name[2]    = 0;
        header->file_name[3]    = 0;
        header->next            = 0;

        // Write in header
        write(fileDescriptor, header, sizeof(*header));
    }
    // verify the file is an archive
    //
    verifyArchive(fileDescriptor);
    // archives already created, nothing to do

}

int main(int argc, char** argv)
{
    int retValue;
    char action;

    if (argc < 3)
    {

        retValue = (argc == 1) ? 0 : 1;
        print_help(retValue);
        return retValue;
    }

    action = parseActions(argv);

    switch(action)
    {
        case 'a':
            // Append
            if (createArchive(argv[2]))
            {
                if (argc >= 4)
                {
                    // Add files to archive

                }
                //
            }
            else
            {
                print("Corrupted Archive, bailing...\n");
            }

            break;
        case 'd':
            createArchive(argv[2]);
            // Delete file
            break;
        default:
            print_help(1);
            return 1;
            break;
    }
}
