#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>

int main(){
    init();

    info(BEGIN, 1, 0);
    
    int pid2 = fork();
    if (pid2 == 0)///child 
    {
        ///process PARENT 2
        info(BEGIN, 2, 0);
        int pid4 = fork();
        if (pid4 == 0)
        {
            ///process parent 4
            info(BEGIN, 4, 0);
            int pid7 = fork();
            if (pid7 == 0)
            {
                ///process 7
                info(BEGIN, 7, 0);

                info(END, 7, 0);
                exit(0);
            }
            else
            {
                //process 4 code
                wait(NULL);
                info(END, 4, 0);
                exit(0);
            }
        }
        else
        {
            ///process 2 parent code
            int pid5 = fork();
            if (pid5 == 0)
            {
                ///process 5
                info(BEGIN, 5, 0);

                info(END, 5, 0);
                exit(0);
            }
            else
            {
                ///process 2 code
                wait(NULL);
                wait(NULL);
                info(END, 2, 0);
                exit(0);
            }
        }
    }
    else
    {
        ///process 1 parent code
        int pid3 = fork();
        if (pid3 == 0)
        {
            ///process 3
            info(BEGIN, 3, 0);
            int pid6 = fork();
            if (pid6 == 0)
            {
                ///process 6
                info(BEGIN, 6, 0);

                info(END, 6, 0);
                exit(0);
            }
            else
            {
                ///process 3 parent code
                int pid8 = fork();
                if (pid8 == 0)
                {
                    ///process 8
                    info(BEGIN, 8, 0);


                    info(END, 8, 0);
                    exit(0);
                }
                else
                {
                    ///process 3 code
                    wait(NULL);
                    wait(NULL);
                    info(END, 3, 0);
                    exit(0);
                }

            }
        }
        else
        {
            wait(NULL); //p2
            wait(NULL);//dupa p3 asteapta
            info(END, 1, 0);
        }
    }
    return 0;
}
