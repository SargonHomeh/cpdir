#include <stdlib.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096
#define LOCATION_SRC 0
#define LOCATION_DST 1

void copydirectory(char * source, char * destination);
void copyfile(char * source, char * destination);
char * makepath(char * location, struct dirent * ep);
void throwerror(char * arg);

int main(int argc, const char * argv[]) {
    // Command line input
    if(argc != 3)
        printf("usage: cpdir <source> <destination>\n");
    else
        copydirectory((char*)argv[1], (char*)argv[2]);
    
    return 0;
}

// copydirectory ( source, directory )
// char * source
//      points to a string containing the source directory location
// char * destination
//      points to a string containing the destination directory location
//
// Description
// ---------------------------------------------------------------
// copies the whole directory from a source location to a destination location, also maintains the directory structure and folder access modes.
void copydirectory(char * source, char * destination)
{
    DIR * dir;
    struct dirent * ep;
    char * location[2];
    struct stat info;
    
    // Open destination directory if exists else create
    if((dir = opendir(destination)) == NULL)
        if(mkdir(destination, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
            throwerror(destination);
    
    // Retrieve destination directory information
    if(stat(source, &info) < 0)
        throwerror(source);
    
    // Open source directory
    if((dir = opendir(source)) == NULL)
        throwerror(source);
    else
    {
        while((ep = readdir(dir)))
        {
            // Directory
            if( (ep->d_type == DT_DIR) && (ep->d_name[0] != '.') )
            {
                // Store directory locations
                location[LOCATION_SRC] = makepath(source, ep);
                location[LOCATION_DST] = makepath(destination, ep);
                copydirectory(location[LOCATION_SRC], location[LOCATION_DST]);
            }
            
            // File
            if(ep->d_type == DT_REG)
            {
                // Store file locations
                location[LOCATION_SRC] = makepath(source, ep);
                location[LOCATION_DST] = makepath(destination, ep);
                copyfile(location[LOCATION_SRC], location[LOCATION_DST]);
            }
        }
    
        // Close folder handle
        if(closedir(dir) < 0)
            throwerror("Error closing folder.");
    }
}

// copyfile ( source, directory )
// char * source
//      points to a string containing the source file location
// char * destination
//      points to a string containing the destination file location
//
// Description
// ---------------------------------------------------------------
// copies a single file from a source location to a destination location, also maintains file access modes.
void copyfile(char * source, char * destination)
{
    int file[2];
    ssize_t nread;
    char buffer[BUFFER_SIZE];   // File data buffer
    
    // Open source file
    if((file[LOCATION_SRC] = open(source, O_RDONLY)) < 0)
        throwerror(source);
    else
    {
        // Retrieve source file information
        struct stat info;
        
        if(stat(source, &info) < 0)
            throwerror(source);
        
        // Create destination file
        if(creat(destination, info.st_mode) < 0)
            throwerror(destination);
        else
        {
            // Open destination file
            if((file[LOCATION_DST] = open(destination, O_WRONLY)) < 0)
                throwerror(destination);
            else
            {
                // Write data to destination file
                while ((nread = read(file[LOCATION_SRC], buffer, BUFFER_SIZE)) > 0)
                    if(write(file[LOCATION_DST], buffer, nread) != nread)
                        //throwerror(destination, ": Error writting destination file.");
                
                    if(nread < 0)
                        throwerror(source);
                
                // Close file handles
                if(close(file[LOCATION_SRC]) < 0 || close(file[LOCATION_DST]) < 0)
                    throwerror("Error closing file.");
            }
        }
    }
}

// makepath ( location, ep )
// char * location
//      points to a string containing the path to construct
// struct dirent * ep
//      contains information about the directory entry
//
// Description
// ---------------------------------------------------------------
// constructs an absolute path for files and folders.
char * makepath(char * location, struct dirent * ep)
{
    // Declare and allocate path space
    char * path = (char *) malloc(1028);

    strcpy(path, location);

    // Add trailing space to root directory
    if(location[strlen(location) - 1] != '/')
        strcat(path, "/");
    
    strcat(path, ep->d_name);

    // Add trailing slash to directory
    if(ep->d_type == DT_DIR)
        strcat(path, "/");
    
    return path;
}

// throwerror ( arg )
// char * arg
//      string to display
//
// Description
// ---------------------------------------------------------------
// throw a error when a system call fails
void throwerror(char * arg)
{
    fprintf(stderr,"cd: %s: ", arg);
    perror("");
}
