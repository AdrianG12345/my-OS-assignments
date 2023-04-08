#include <stdio.h>
#include <stdlib.h>
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
int wanted_section;
int wanted_line;

int nr_test;



void add(char *sir, char* added, int line)
{

    int len = strlen(sir);
    strcpy(sir + len, added);
    if (line)
    {
        len = strlen(sir);
        strcpy(sir + len, "\n");
    }

}
void add2(char* sir, char* added)
{

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
                        add(output, name, 1);
                    }
                    else if (has_perm_write == 0)
                        add(output, name, 1);
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
                        add(output, name, 1);
                    }
                    else if (has_perm_write == 0)
                    {
                        add(output, name, 1);
                    }
                }    
            }
        }       
    } 
    closedir(dir);
}

void parsare(char* path )
{
    int fd;
    if (! (fd = open(path, O_RDONLY)))
    {
        ok = -1;
        char* chestie = "wrong file";
        add(eroare, chestie, 1);
        return;
    }

    char magic[3];
    lseek(fd, -2, SEEK_END);
    read(fd, magic, 2);
    if (magic[0] != 'Q' || magic[1] != 'p')
    {
        ok = -1;
        char* chestie = "wrong magic";
        add(eroare, chestie, 1);
        return;
    }

    //char buffer[8];

    int header_size = 0;
    lseek(fd, -4, SEEK_END);
    read(fd, &header_size, 2);
    
    lseek(fd, -header_size, SEEK_END);
    int version = 0;
    int nr_sections = 0;
    read(fd, &version, 2);
    read(fd, &nr_sections, 1);

    if (! (version >= 120 && version <= 176))
    {
        //printf("%d", version);
        ok = -1;
        char* chestie = "wrong version";
        add(eroare, chestie, 1);
        return;
    }

    if (! (nr_sections >= 4 && nr_sections <= 18))
    {
        ok = -1;
        char* chestie = "wrong sect_nr";
        add(eroare, chestie, 1);
        return;
    }

    //78 63 40 17 44
    /*version=<version_number>
    nr_sections=<no_of_sections>
    section1: <NAME_1> <TYPE_1> <SIZE_1>
    section2: <NAME_2> <TYPE_2> <SIZE_2>*/
    /*
        SECT_NAME: 6
    SECT_TYPE: 4
    SECT_OFFSET: 4
    SECT_SIZE: 4
    */
    int type[100],size[100],offset[100];
    char name[1000][1000];
    for (int i = 1; i <= nr_sections; i++)
    {
        read(fd, &name[i], 6);
        read(fd, &type[i], 4);
        read(fd, &offset[i], 4);
        read(fd, &size[i], 4);

        if (! (type[i] == 78 || type[i] == 63 || type[i] == 40 || type[i] == 17 || type[i] == 44) )
        {
            ok = -1;
            add(eroare, "wrong sect_types", 1);
            return;
        }
    }

    
    printf("SUCCESS\n");
    printf("version=%d\n", version);
    printf("nr_sections=%d\n", nr_sections);
    for (int i = 1; i <= nr_sections; i++)
    {
        printf("section%d: %s %d %d\n", i, name[i], type[i], size[i]);
    }
}

void extract(char *path, int ws, int w_line)
{
    int fd;
    if (! (fd = open(path, O_RDONLY)))
    {
        ok = -1;
        char* chestie = "invalid file";
        add(eroare, chestie, 1);
        return;
    }

    char magic[3];
    lseek(fd, -2, SEEK_END);
    read(fd, magic, 2);

    int header_size = 0;
    lseek(fd, -4, SEEK_END);
    read(fd, &header_size, 2);
    
    lseek(fd, -header_size, SEEK_END);
    int version = 0;
    read(fd, &version, 2);

    int nr_sections = 0;
    read(fd, &nr_sections, 1);


    int section_size = 18;
    lseek(fd, (ws - 1) * section_size, SEEK_CUR);

        int type,offset,size;
    char name[100000];

    // for (int i = 1; i < ws; i++)
    // {
    //     read(fd, &name, 6);
    //     read(fd, &type, 4);
    //     read(fd, &offset, 4);
    //     read(fd, &size, 4);
    // }


    read(fd, &name, 6);
    read(fd, &type, 4);
    read(fd, &offset, 4);
    read(fd, &size, 4);

    //printf("%s %d %d %d", name, type, offset, size);
    
    char* buffer;
    buffer = malloc( (size + 20) * sizeof(char));
    lseek(fd, offset, SEEK_SET);
    read(fd, buffer, size);

    int linie = 1;

    char* ceva;
    ceva = malloc( (size + 20) * sizeof(char));
    int len = 0;
    
    for (int i = size - 1; i >= 0; i--)
    {
        if (buffer[i] == 13 && buffer[i + 1] == 10)////din pdf
          linie++;
        else if (linie == w_line)
        {
            ceva[len++] = buffer[i];
        }
        else if (linie > w_line)
             break;
    }
    //printf("%d\n   ", linie);
    
    if (linie < w_line)
    {
        ok = -1;
        add(eroare, "invalid line", 1);
        return;
    }

    printf("SUCCESS");
    for (int i = len - 1; i >= 0; i--)
        printf("%c", ceva[i]);
    printf("\n");




    free(ceva);
    free(buffer);


    // int MAX_SIZE = 5000;
    // char buffer[MAX_SIZE + 10];
    // int i = 0;
    // int linie = 1;
    // int ramas, citit;

    // //int len = 0;
    // while (i < size)
    // {
    //     ramas = size - i;
    //     if (ramas >= MAX_SIZE)
    //     {
    //         i += MAX_SIZE;
    //         read(fd, &buffer, MAX_SIZE);
    //         citit = MAX_SIZE;
    //     }
    //     else
    //     {
    //         read(fd, &buffer, ramas);
    //         i += ramas;
    //         citit = ramas;
    //     }

    //     for (int i = 0; i < citit; i++)
    //     {
    //         if (buffer[i] == 13 && buffer[i + 1] == 10)////din pdf
    //             linie++;
    //         else if (linie == w_line)
    //         {
    //             //chestie[len++] = buffer[i];
    //             printf("%c", buffer[i]);
    //         }
    //         if (linie > w_line)
    //         {
    //             printf("\n");;
    //             return;
    //         }
    //     }
    //}


}

int check_14(char* path, long long file_size)
{
    int fd;
    if (! (fd = open(path, O_RDONLY)))
    {
        //ok = -1;
        //char* chestie = "wrong file";
        printf("wrong file %s\n", path);
        //add(eroare, chestie, 1);
        return -1;
    }

    char magic[3];
    lseek(fd, -2, SEEK_END);
    read(fd, magic, 2);

    int header_size = 0;
    lseek(fd, -4, SEEK_END);
    read(fd, &header_size, 2);
    
    lseek(fd, -header_size, SEEK_END);
    int version = 0;
    int nr_sections = 0;
    read(fd, &version, 2);
    read(fd, &nr_sections, 1);
    /*VERSION: 2
NO_OF_SECTIONS: 1
SECTION_HEADERS: NO_OF_SECTIONS * sizeof(SECTION_HEADER)
SECTION_HEADER: 6 + 4 + 4 + 4
SECT_NAME: 6
SECT_TYPE: 4
SECT_OFFSET: 4
SECT_SIZE: 4
HEADER_SIZE: 2
MAGIC: 2*/
    //78 63 40 17 44
    /*version=<version_number>
    nr_sections=<no_of_sections>
    section1: <NAME_1> <TYPE_1> <SIZE_1>
    section2: <NAME_2> <TYPE_2> <SIZE_2>*/
    /*
        SECT_NAME: 6
    SECT_TYPE: 4
    SECT_OFFSET: 4
    SECT_SIZE: 4
    */
    int type[100],size[100],offset[100];
    char name[1000];

    char* buffer;
    

    int nr_mare = 2000000000;
    buffer = malloc( (nr_mare + 20) * sizeof(char));
    //printf("nr_sections%d\n", nr_sections);
    for (int i = 1; i <= nr_sections; i++)
    {
        type[i] = size[i] = offset[i] = 0;
        int nr = read(fd, &name, 6);
        if (nr == -1 || nr == 0)
            return 0;

        //printf("name ");
        nr = read(fd, &type[i], 4);
        if (nr == -1 || nr == 0)
            return 0;

        //printf("type ");
        nr = read(fd, &offset[i], 4);
        if (nr == -1 || nr == 0)
            return 0;

        //printf("offset ");
        nr = read(fd, &size[i], 4);
        if (nr == -1)
            return 0;

        //printf("size\n");
        
        
        // if (size > nr_mare)
        //     continue;
        // printf("section:%d %d %d\n",i, offset, size);

        // if (offset > file_size)
        //     continue; 
        
        //printf("offset bun\n");
        
        // lseek(fd, offset[i], SEEK_SET);
        // if ( read(fd, buffer, size) == -1)
        //     continue;
        // int linie = 1;
        
        
        // for (int k = size - 1; k >= 0; k--)
        // {
        //     if (buffer[k] == 13 && buffer[k + 1] == 10)////din pdf
        //     {
        //         linie++;
        //         if (linie == 14)
        //         {
        //             free(buffer);
        //             printf("okay\n\n");
        //             return 1;
        //         }
        //     }
        // }  
    }
    for (int i = 1; i <= nr_sections; i++)
    {
        if (offset[i] > file_size)
            continue;

        int linie = 1;
        
        lseek(fd, offset[i], SEEK_SET);
        read(fd, buffer, size[i]);
        for (int k = size[i] - 1; k >= 0; k--)
        {
            if (buffer[k] == 13 && buffer[k + 1] == 10)////din pdf
            {
                linie++;
                if (linie == 15)
                {
                    free(buffer);
                    return 1;
                }
            }
        }  
    }
    free(buffer);

    return 0;
}


void findall(char* path)
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
                findall(name);
                    
            }
            else //if (S_ISREG(inode.st_mode))
            {      
                  //  printf("\n\n%s %ld\n",name,  inode.st_size);
                  int check = check_14(name, inode.st_size);
                  if (check == 1)
                  {
                    nr_test++;
                    ///adaug acest fisier
                    add(output, name, 1);
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
    
     if (strcmp(argv[1], "parse") == 0)
    {
        char path[100000];
        strcpy(path, "ceva");
        if ( strncmp(argv[2], "path=", strlen("path=")) == 0)
                strcpy(path, argv[2] + 5);
        else
        {
            printf("%sinvalid path", eroare);
            return 0;
        }

        parsare(path);
        if (ok == -1)
        {
            printf("%s", eroare);
        }
        return 0;
    }

    if (strcmp(argv[1], "extract") == 0)
    {
        char path[100000];
        strcpy(path, "ceva");
        add(output, "SUCCESS", 1);
        for (int i = 2; i < argc; i++)
        {
            if ( strncmp(argv[i], "path=", strlen("path=")) == 0)
                strcpy(path, argv[i] + 5);
            else if ( strncmp(argv[i], "section=", strlen("section=") ) == 0)
            {
                wanted_section = 0;
                for (int j = strlen("section="); j < strlen(argv[i]); j++)
                {
                    wanted_section *= 10;
                    wanted_section += argv[i][j] - '0';
                }
            }
            else if (strncmp(argv[i], "line=", strlen("line=")) == 0 )
            {
                wanted_line = 0;
                for (int j = strlen("line="); j < strlen(argv[i]); j++)
                {
                    wanted_line *= 10;
                    wanted_line += argv[i][j] - '0';
                }
            }
        }
        extract(path, wanted_section, wanted_line);

        if (ok == -1)
        {
            printf("%s", eroare);
        }

        return 0;
        
    }
    if (strcmp(argv[1], "findall") == 0)
    {
        char path[10000];
        strcpy(path, argv[2] + 5);

        struct stat fileMetaData;
        if ( stat(path, &fileMetaData) < 0)
        {
            char* chestie = "invalid directory path\n";          
            printf("%s", eroare);
            printf("%s", chestie);

            return 0;
            //exit(2);
        }

        findall(path);
        printf("%s", output);
       // printf("%d\n", nr_test);

        return 0;
    }
    
    return 0;
}
