#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <sys/mman.h>


int reqPipe, respPipe;

void createSHM()
{
    ///read that weird number first


    char* name = "/yp9Eqg";
    int size = 4534755;
    int sharedMemory = shm_open(name, O_CREAT, 664);
    if (sharedMemory == -1){
        perror("FAILURE TO MAKE SHARED MEMORY");
    }

    if (ftruncate(sharedMemory, size) == -1) {
        perror("Failed to set size");
        exit(1);
    }

    ///3rd paramter: protection (read | write)
    void* sharedMemoryPtr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, sharedMemory, 0);
    if (sharedMemoryPtr == MAP_FAILED) {
        perror("Failed to map the shared memory region");
        exit(1);
    }
    munmap(sharedMemoryPtr, size);
    close(sharedMemory);
    shm_unlink(name);
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

        if (strncmp(buffer, "VARIANT", sizeof("VARIANT")) == 0)
        {
            /// 27BC == 10172
            char *varianta = "VARIANT!10172VALUE!";
            write(respPipe, varianta, sizeof(varianta));
            break;
        }
        else if (strncmp(buffer, "CREATE_SHM", sizeof("CREATE_SHM")) == 0)
        {
            createSHM();
        }
        else if (strncmp(buffer, "WRITE_TO_SHM", sizeof("WRITE_TO_SHM")) == 0)
        {

        }
        else if (strncmp(buffer, "MAP_FILE", sizeof("MAP_FILE")) == 0)
        {
        }
        else if (strncmp(buffer, "READ_FROM_FILE_OFFSET", sizeof("READ_FROM_FILE_OFFSET")) == 0)
        {
        }
        else if (strncmp(buffer, "READ_FROM_FILE_SECTION", sizeof("READ_FROM_FILE_SECTION")) == 0)
        {
        }
        else if (strncmp(buffer, "READ_FROM_LOGICAL_SPACE_OFFSET", sizeof("READ_FROM_LOGICAL_SPACE_OFFSET")) == 0)
        {
        }
        else if (strncmp(buffer, "EXIT", sizeof("EXIT")) == 0)
        {
            break;
        }
        ok = 0;
    }

    // Close the pipes
    close(reqPipe);
    close(respPipe);

    // Remove the response pipe
    unlink(responsePipeString);

    return 0;
}