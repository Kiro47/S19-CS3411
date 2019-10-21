#include "utils.h"
#include "tar.h"

#include <stdlib.h>
#include <string.h>
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
void addHeader(int fileDescriptor);
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
    int verification;
    off_t offset;

    existStatus = doesFileExist(filename);
    if (existStatus <= 0)
    {
        return existStatus;
    }
    fileDescriptor = open(filename, O_RDWR|O_CREAT, 0644);
    // file descriptor shouldn't have the option of being neg due to above

    offset = lseek(fileDescriptor, 0, SEEK_END);
    if (offset == 0)
    {
        // file is empty, construct header
        addHeader(fileDescriptor);
    }
    // verify the file is an archive
    verification = verifyArchive(fileDescriptor);
    close(fileDescriptor);
    return verification;
}

/*
 * addHeader(int fileDescriptor)
 *      Adds a new header to the file of fileDescriptor
 *      Always adds to end of file
 *  fileDescriptor: Descritptor of file to add too
 */
void addHeader(int fileDescriptor)
{
    off_t offset;
    tarHeader *header;

    // go to end of file
    offset = lseek(fileDescriptor, 0, SEEK_END);

    // allocate
    header = malloc(sizeof(*header));
    // Construct
    header->magic           = TAR_MAGIC_VAL;
    header->eop             = offset + sizeof(*header);
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
    // Free
    free(header);
}

/*
 * addFile(char *archiveName, char *filename)
 *      Adds the file filename to the archive
 *
 *  archiveName: Name of the archive to add to
 *  filename:    name of the file to add
 *
 *  returns:
 *      ?
 */
int addFile(char *archiveName, char *filename)
{
    int existStatus;
    int fdNewFile;
    int fdArchive;
    int i;
    int namePointer;
    off_t headerLocation;
    off_t archiveOffset;
    off_t fileOffset;
    int nextHeader;
    char *filenameTerm;

    tarHeader *header;

    existStatus = doesFileExist(filename);
    if (existStatus)
    {
        return existStatus;
    }
    // fd = fileDescriptor
    // File exists and is readable
    fdNewFile = open(filename, O_RDONLY, 0644);
    fdArchive = open(archiveName, O_RDWR, 0644);

    header = malloc(sizeof(*header));

    // Open archive and get header
    headerLocation = lseek(fdArchive, 0, SEEK_SET);

    read(fdArchive, header, sizeof(*header));

    do
    {
        // 4 hard coded from maximum files per header
        for (i = 0; i < 4; i++)
        {
            if (header->file_name[i] == 0)
            {
                // Use this index as next file header
                break;
            }
        }
        if (i >= 4)
        {
            nextHeader = findNextHeader(fdArchive);

            if (nextHeader == -2)
            {
                // make new header
                addHeader(fdArchive);
                i = 0;
                break;
            }
            else
            {
                headerLocation = lseek(fdArchive, nextHeader, SEEK_SET);
            }
            read(fdArchive, header, sizeof(*header));
        }
        // just use the header
        break;
    } while (header->next != 0);

    // TODO: modify header
    headerLocation = lseek(fdArchive, sizeof(*header) * -1, SEEK_CUR);
    archiveOffset = lseek(fdArchive, 0, SEEK_END);
    namePointer = lseek(fdArchive, 0, SEEK_CUR);
    // Write in filename
    filenameTerm = malloc((strlen(filename) + 1) * sizeof(char));
    strcpy(filenameTerm, filename);
    filenameTerm[strlen(filename) + 1] = 0x0;
    write(fdArchive, filenameTerm, (strlen(filenameTerm) * sizeof(char)));

    // Copy file
    // construct header
    header->magic           = TAR_MAGIC_VAL;
    // namePointer hasn't moved yet, so we can abuse that
    header->file_name[i]    = namePointer;
    header->file_size[i]    = copyToArchive(fdNewFile, fdArchive, (namePointer
                + (sizeof(char) * strlen(filenameTerm) + 1)));
    header->eop             = lseek(fdArchive, 0, SEEK_END);
    header->block_count    += 1;
    // Write back in header
    lseek(fdArchive, headerLocation, SEEK_SET);
    write(fdArchive, header, sizeof(*header));
    // Clean up
    free(header);
    free(filenameTerm);
    close(fdArchive);
    close(fdNewFile);
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
            if (createArchive(argv[2]) >= 0)
            {
                if (argc >= 4)
                {
                    // Add files to archive
                    int i;
                    // Start at firt file to add name
                    for (i = 3; i < argc; i++)
                    {
                        addFile(argv[2],argv[i]);
                    }
                }
                else
                {
                    print("Created archive %s\n", argv[2]);
                }
                //
            }
            else
            {
                print("Corrupted Archive or error creating, bailing...\n");
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
