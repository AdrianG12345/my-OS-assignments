#include <stdio.h>
#include <string.h>
#include  <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>



#define MAX_PATH_LEN 1024


int recursive;
int ok;
char output[1000000];
char eroare[100000];



void add(char *sir, char* added)
{

    int len = strlen(sir);
    strcpy(sir + len, added);
    len = strlen(sir);
    strcpy(sir + len, "\n");
}


void listare(char* path, int size_greater, int has_perm_write)
{
    DIR* dir;
    struct dirent *dirEntry;
    struct stat inode;
    char name[MAX_PATH_LEN];

    dir = opendir(path);
    if (dir == 0)
    {
        return;
    }

    while ( (dirEntry = readdir(dir)) != 0)
    {
       if ( strcmp(dirEntry->d_name, ".") == 0|| strcmp(dirEntry->d_name, "..") == 0 )
            continue; 

        snprintf(name, MAX_PATH_LEN, "%s/%s", path, dirEntry->d_name);

        if (! lstat(name, &inode))
        {
            if (S_ISDIR(inode.st_mode))
            {
                if (inode.st_size > size_greater)
                {
                    if ( has_perm_write == 1 &&  (inode.st_mode & S_IWUSR) != 0)
                    {
                        add(output, name);
                    }
                    else if (has_perm_write == 0)
                        add(output, name);
                }

                if (recursive == 1)
                {
                    
                    if ( has_perm_write == 1 &&  (inode.st_mode & S_IWUSR) != 0)
                    {
                        listare(name, size_greater, has_perm_write);
                    }
                    else if (has_perm_write == 0)
                    {
                        listare(name, size_greater, has_perm_write);
                    }
                }
                    
            }
            else if (S_ISREG(inode.st_mode))
            {      
                if (inode.st_size > size_greater)
                {
                    if ( has_perm_write == 1 &&  (inode.st_mode & S_IWUSR) != 0)
                    {
                        add(output, name);
                    }
                    else if (has_perm_write == 0)
                    {
                        add(output, name);
                    }
                }    
            }
        }       
    } 
    closedir(dir);
}


int main(int argc, char** argv)
{
    strcpy(output, "SUCCESS\n");
    strcpy(eroare, "ERROR\n");
    if (argc < 2)
    {
        ///error
        printf("ERROR\nNOT ENOUGH PARAMTERES\n");
        return -1;
    }

    if (strcmp(argv[1], "variant") == 0)///is la fel
    {
        printf("10172\n");
        return 0;
    }

    if (strcmp(argv[1], "list") == 0)
    {
        ok = 1;

        char path[100000];
        strcpy(path, "ceva");
        int size_greater = -1;
        int has_perm_write = 0;
        for (int i = 2; i < argc; i++)
        {
            if ( strncmp(argv[i], "recursive", strlen("recursive"))  == 0 )
                recursive = 1;
            else if (strncmp(argv[i], "size_greater=", strlen("size_greater=")) == 0 )
            {
                size_greater = 0;
                for (int j = strlen("size_greater="); j < strlen(argv[i]); j++)
                {
                    size_greater *= 10;
                    size_greater += argv[i][j] - '0';
                }
            }
            else if ( strncmp(argv[i], "has_perm_write", strlen("has_perm_write")) == 0 )
                has_perm_write = 1;
            else if ( strncmp(argv[i], "path=", strlen("path=")) == 0)
                strcpy(path, argv[i] + 5);
        }
        //printf("%d ", has_perm_write);
        struct stat fileMetaData;
        if ( stat(path, &fileMetaData) < 0)
        {
            char* chestie = "invalid directory path\n";          
            printf("%s", eroare);
            printf("%s", chestie);
            //exit(2);
        }
        else
        {
            listare(path, size_greater, has_perm_write);
            printf("%s", output);
        }
        
        return 0;
    }
    
    return 0;
}