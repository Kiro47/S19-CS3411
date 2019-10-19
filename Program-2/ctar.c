#include "utils.h"
#include <unistd.h>

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
 *      int: 0 if created and ready, otherwise error code from opening.
 */
int createArchive(char* filename)
{
    int existStatus;
    int fileDescriptor;

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
    //
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
            createArchive(argv[2]);

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
