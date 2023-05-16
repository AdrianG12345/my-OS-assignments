#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>
// int pageSize =  sysconf(_SC_PAGESIZE);
//     if (pageSize == -1){
//         perror("FAIL HERE PAGE SIZE");
//     }
// int adjustedSize = ((size + pageSize - 1) / pageSize) * pageSize;

int reqPipe, respPipe;
void *sharedMemoryPtr;

int createSHM()
{
    char *name = "/yp9Eqg";
    int size = 4534755;
    int sharedMemory = shm_open(name, O_CREAT | O_RDWR, 664);
    if (sharedMemory == -1)
    {
        perror("FAILURE TO MAKE SHARED MEMORY");
        return 1;
    }

    if (ftruncate(sharedMemory, size) == -1)
    {
        perror("Failed to set size with ftruncate");
        return 1;
    }

    /// 3rd paramter: protection (read | write)
    sharedMemoryPtr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemory, 0);
    if (sharedMemoryPtr == MAP_FAILED)
    {
        perror("Failed to map the shared memory region");
        return 1;
    }
    return 0;
    // munmap(sharedMemoryPtr, size);
    // close(sharedMemory);
    // shm_unlink(name);

    // if (munmap(sharedMemoryPtr, size) == -1) {
    //     perror("Failed to unmap the shared memory region");
    // }

    // // Close the shared memory file descriptor
    // if (close(sharedMemory) == -1) {
    //     perror("Failed to close the shared memory file descriptor");
    // }

    // // Remove the shared memory object
    // if (shm_unlink(name) == -1) {
    //     perror("Failed to remove the shared memory object");
    // }
}
int mapFile(char *path)
{

    int fd = open(path, O_RDONLY);
    if (fd == -1)
    {
        return 1;
    }
    struct stat st;
    if (fstat(fd, &st) == -1)
    {
        return 1;
    }
    off_t fileSize = st.st_size;

    void *filePtr = mmap(NULL, fileSize, PROT_READ, MAP_PRIVATE, fd, 0);
    if (filePtr == MAP_FAILED)
    {
        return 1;
    }

    return 0;
}

int main()
{
    char *requestPipeString = "./REQ_PIPE_10172";
    char *responsePipeString = "./RESP_PIPE_10172";

    // Step 1: Create the response pipe
    if ((mkfifo(responsePipeString, 0666)) == -1)
    {
        printf("ERROR\ncannot create the response pipe");
        return 1;
    }

    // Step 2: Open the request pipe for reading
    reqPipe = open(requestPipeString, O_RDONLY);

    if (reqPipe < 0)
    {
        printf("ERROR\ncannot open the request pipe");
        unlink(responsePipeString);
        return 2;
    }

    // Step 3: Open the response pipe for writing
    respPipe = open(responsePipeString, O_WRONLY);

    if (respPipe < 0)
    {
        printf("ERROR\ncannot open the response pipe");
        unlink(responsePipeString);
        return 3;
    }
    // Step 4: connect
    char *conectare = "CONNECT!";
    if ((write(respPipe, conectare, sizeof(conectare))) == -1)
    {
        unlink(responsePipeString);
        return 4;
    }
    // write(respPipe, conectare, sizeof(conectare));

    /// step 4 write success
    printf("SUCCESS\n");

    // char c;
    char buffer[256];
    int len;
    int ok = 1;
    while (ok) // until break
    {
        len = 0;
        read(reqPipe, &buffer[len], sizeof(char)); /// nu citeste nici macar aici;
        // buffer[len] = c;
        while (buffer[len] != '!')
        {
            len++;
            read(reqPipe, &buffer[len], sizeof(char));
            // buffer[len] = c;
        }

        if (strncmp(buffer, "VARIANT", strlen("VARIANT")) == 0)
        {
            /// 00 00 27 BC == 10172
            /// CB 72
            /// BC 27
            char *varianta = "VARIANT!\0";
            for (int i = 0; i < strlen(varianta); i++)
                write(respPipe, &varianta[i], 1);

            char arr[4];
            arr[2] = 0;
            arr[3] = 0;
            arr[0] = 188; /// BC
            arr[1] = 39;  /// 27
            for (int i = 0; i < 4; i++)
                write(respPipe, &arr[i], 1);
            char *varianta1 = "VALUE!\0";
            for (int i = 0; i < strlen(varianta1); i++)
                write(respPipe, &varianta1[i], 1);
            break;
        }
        else if (strncmp(buffer, "CREATE_SHM", strlen("CREATE_SHM")) == 0)
        {
            read(reqPipe, buffer, 4);
            int k = createSHM();
            if (k == 0)
            {
                char *msg = "CREATE_SHM!SUCCESS!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }
            else
            {
                char *msg = "CREATE_SHM!ERROR!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }
        }
        else if (strncmp(buffer, "WRITE_TO_SHM", strlen("WRITE_TO_SHM")) == 0)
        {
            unsigned int offset = 0;
            unsigned int val;
            read(reqPipe, &offset, sizeof(offset));
            read(reqPipe, &val, sizeof(val));
            // printf("%d %d\n", offset, val);
            int size = 4534755;
            if (sharedMemoryPtr == NULL || offset < 0 || offset + 4 > size) // cazuri de eroare
            {
                /// NU AM SHARED MEMORY sau unde sa scriu
                char *msg = "WRITE_TO_SHM!ERROR!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }
            else
            {
                void *newLocation = sharedMemoryPtr + offset;
                memcpy(newLocation, &val, sizeof(val));
                char *msg = "WRITE_TO_SHM!SUCCESS!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }
        }
        else if (strncmp(buffer, "MAP_FILE", strlen("MAP_FILE")) == 0)
        {
            char path[256];
            int lungime = 0;
            read(reqPipe, &path[lungime], sizeof(char)); /// nu citeste nici macar aici;
            while (path[lungime] != '!')
            {
                lungime++;
                read(reqPipe, &path[lungime], sizeof(char));
            }
            // path[lungime] == '!';
            path[lungime] = '\0';
            int k = mapFile(path);
            if (k == 0)
            {
                char *msg = "MAP_FILE!SUCCESS!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }
            else
            {
                char *msg = "MAP_FILE!ERROR!\0";
                for (int i = 0; i < strlen(msg); i++)
                    write(respPipe, &msg[i], 1);
            }

            ok = 0;
        }
        else if (strncmp(buffer, "READ_FROM_FILE_OFFSET", strlen("READ_FROM_FILE_OFFSET")) == 0)
        {
            ok = 0;
        }
        else if (strncmp(buffer, "READ_FROM_FILE_SECTION", strlen("READ_FROM_FILE_SECTION")) == 0)
        {
            ok = 0;
        }
        else if (strncmp(buffer, "READ_FROM_LOGICAL_SPACE_OFFSET", strlen("READ_FROM_LOGICAL_SPACE_OFFSET")) == 0)
        {
            ok = 0;
        }
        else if (strncmp(buffer, "EXIT", strlen("EXIT")) == 0)
        {
            ok = 0;
            break;
        }
    }

    // Close the pipes
    close(reqPipe);
    close(respPipe);

    // Remove the response pipe
    unlink(responsePipeString);

    return 0;
}