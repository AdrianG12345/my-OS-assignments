#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "a2_helper.h"
#include <stdlib.h>
#include <pthread.h>

#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <fcntl.h>

#include <semaphore.h>

int semID;
int incepe5 = 0;
int incepe4 = 0;
sem_t sem1, sem2;

sem_t semnal2;
int count;
pthread_mutex_t lock, lacat, lacat1;
pthread_cond_t cond, condition, condition1;

void destroy_them()
{
    if (pthread_mutex_destroy(&lacat) != 0)
    {
        perror("Cannot destroy the lock");
        
    }

    if (pthread_cond_destroy(&condition) != 0)
    {
        perror("Cannot destroy the condition variable");
        
    }

    if (pthread_mutex_destroy(&lacat1) != 0)
    {
        perror("Cannot destroy the lock");
        
    }

    if (pthread_cond_destroy(&condition1) != 0)
    {
        perror("Cannot destroy the condition variable");
        
    }
}
void create_them()
{
    if (pthread_mutex_init(&lacat, NULL) != 0)
    {
        perror("Cannot initialize the lock");
       
    }
    if (pthread_cond_init(&condition, NULL) != 0)
    {
        perror("Cannot initialize the condition variable");
        
    }
    if (pthread_mutex_init(&lacat1, NULL) != 0)
    {
        perror("Cannot initialize the lock");
        
    }
    if (pthread_cond_init(&condition, NULL) != 0)
    {
        perror("Cannot initialize the condition variable");
       
    }
}

void P(int semId, int semNr) /// din text obligatoriu pt laboratoare
{
    struct sembuf op = {semNr, -1, 0};
    semop(semId, &op, 1);
}

void V(int semId, int semNr)
{
    struct sembuf op = {semNr, 1, 0};
    semop(semId, &op, 1);
}

void *functieP3(void *arg)
{
    int id = *(int *)arg;

    if (id == 3) // incepe inainte de 4 si se gata dupa 4
    {
        info(BEGIN, 3, id);

        /// ii zic la 4 ca poate incepe
        sem_post(&sem1);

        /// astept ca 4 sa termine
        //
        sem_wait(&sem2);
        info(END, 3, id);
    }
    else if (id == 4)
    {
        /// cresc semaforul pentru T3.3 ca sa poata sa se termine
        /// incepe dupa 3 se termina inainte de 3

        /// astept 3 sa ma lase sa incep
        sem_wait(&sem1);

        info(BEGIN, 3, id);

        info(END, 3, id);
        /// ii zic la 3 ca se poate termina;
        sem_post(&sem2);
    }
    else if (id == 5)
    {
        /*
        T2.2 trebuie sa fi inceput deja
        */
        P(semID, 1);

        info(BEGIN, 3, 5);

        info(END, 3, 5);

        /*
        T2.4 poate sa inceapa
        */

        V(semID, 0);
    }
    else
    {
        info(BEGIN, 3, id);
        info(END, 3, id);
    }

    free(arg);
    return NULL;
}

void *func(void *arg)
{
    int id = *(int *)arg;
    info(BEGIN, 4, id);

    info(END, 4, id);

    free(arg);
    return NULL;
}

void *func2(void *arg)
{
    int id = *(int *)arg;
    info(BEGIN, 4, id);

    if (id != 13)
    {
        /// ii zic la cel cu 13 ca am inceput
        pthread_mutex_lock(&lock);

        count++;
        pthread_cond_broadcast(&cond);
        pthread_mutex_unlock(&lock);

        /// astept semnal ca sa pot da end
        sem_wait(&semnal2);
        info(END, 4, id);
    }
    else
    {
        /// daca am celelalte 4 gata
        pthread_mutex_lock(&lock);
        while (count != 4)
            pthread_cond_wait(&cond, &lock);

        info(END, 4, id);
        /// nu-s sigur daca trebuie
        pthread_mutex_unlock(&lock);
        /// le zic la celelalte 4 ca se pot termina
        for (int i = 0; i < 4; i++)
            sem_post(&semnal2);
    }

    free(arg);
    return NULL;
}

void *funcP2(void *arg)
{
    int id = *(int *)arg;
    if (id == 2)
    {
        info(BEGIN, 2, 2);
        info(END, 2, 2);

        /*
        ii zice la T3.5 ca poate incepe
        */
        V(semID, 1);
    }
    else if (id == 4)
    {
        /// incepe doar dupa T3.5 s-a terminat

        P(semID, 0);//asteapta sa scada din ala

        // pthread_mutex_lock(&lacat1);
        // while (incepe4 != 1)
        //     pthread_cond_wait(&condition1, &lacat1);

        // pthread_mutex_unlock(&lacat1); /// nu-s sigur daca trebuie aceasta linie

        info(BEGIN, 2, 4);
        info(END, 2, 4);
    }
    else
    {
        info(BEGIN, 2, id);
        info(END, 2, id);
    }

    free(arg);
    return NULL;
}

int main()
{
    init();

    info(BEGIN, 1, 0);

    //create_them();
    semID = semget(IPC_PRIVATE, 2, 0666 | IPC_CREAT);///internet
    semctl(semID, 0, SETVAL, 0);
    semctl(semID, 1, SETVAL, 0);

    int pid2 = fork();
    if (pid2 == 0) /// child
    {
        /// process PARENT 2
        info(BEGIN, 2, 0);
        int pid4 = fork();
        if (pid4 == 0)
        {
            /// process parent 4
            info(BEGIN, 4, 0);

            int pid7 = fork();
            if (pid7 == 0)
            {
                /// process 7
                info(BEGIN, 7, 0);

                info(END, 7, 0);
                exit(0);
            }
            else
            {
                // process 4 code

                pthread_t th[50];

                /// primele 10
                for (int i = 0; i < 2; i++)
                {
                    for (int k = 1; k <= 5; k++)
                    {
                        int *id = malloc(sizeof(int));
                        *id = (i * 5 + k);
                        pthread_create(&th[*id], NULL, func, id);
                        // pthread_join(th[*id], NULL);
                    }
                    for (int k = 1; k <= 5; k++)
                        pthread_join(th[i * 5 + k], NULL);
                }

                /// cel cu cerintele mari
                sem_init(&semnal2, 0, 0);
                count = 0;
                if (pthread_mutex_init(&lock, NULL) != 0)
                {
                    perror("Cannot initialize the lock");
                    exit(2);
                }
                if (pthread_cond_init(&cond, NULL) != 0)
                {
                    perror("Cannot initialize the condition variable");
                    exit(3);
                }

                for (int i = 11; i <= 15; i++)
                { /// T4.13
                    int *id = malloc(sizeof(int));
                    *id = i;
                    pthread_create(&th[*id], NULL, func2, id);
                }

                for (int i = 11; i <= 15; i++)
                    pthread_join(th[i], NULL);

                if (pthread_mutex_destroy(&lock) != 0)
                {
                    perror("Cannot destroy the lock");
                    return 8;
                }

                if (pthread_cond_destroy(&cond) != 0)
                {
                    perror("Cannot destroy the condition variable");
                    return 9;
                }

                // 16 to 35
                for (int i = 3; i <= 6; i++)
                {
                    for (int k = 1; k <= 5; k++)
                    {
                        int *id = malloc(sizeof(int));
                        *id = (i * 5 + k);
                        pthread_create(&th[*id], NULL, func, id);
                        // pthread_join(th[*id], NULL);
                    }
                    for (int k = 1; k <= 5; k++)
                        pthread_join(th[i * 5 + k], NULL);
                }

                /// 36 to 39
                for (int i = 36; i <= 39; i++)
                {
                    int *id = malloc(sizeof(int));
                    *id = i;
                    pthread_create(&th[*id], NULL, func, id);
                    // pthread_join(th[*id], NULL);
                }
                for (int i = 36; i <= 39; i++)
                    pthread_join(th[i], NULL);

                /*rest of code*/
                wait(NULL);
                info(END, 4, 0);
                exit(0);
            }
        }
        else
        {
            /// process 2 parent code
            int pid5 = fork();
            if (pid5 == 0)
            {
                /// process 5
                info(BEGIN, 5, 0);

                info(END, 5, 0);
                exit(0);
            }
            else
            {
                /// process 2 code
                // semID = semget(100000, 2, 0);

                

                pthread_t th[6];
                for (int i = 1; i <= 4; i++)
                {
                    int *id = malloc(sizeof(int));
                    *id = i;
                    pthread_create(&th[i], NULL, funcP2, id);
                }

                wait(NULL);
                wait(NULL);

                for (int i = 1; i <= 4; i++)
                    pthread_join(th[i], NULL);
                info(END, 2, 0);
                exit(0);
            }
        }
    }
    else
    {
        /// process 1 parent code
        int pid3 = fork();
        if (pid3 == 0)
        {
            /// process 3
            info(BEGIN, 3, 0);
            int pid6 = fork();
            if (pid6 == 0)
            {
                /// process 6
                info(BEGIN, 6, 0);

                info(END, 6, 0);
                exit(0);
            }
            else
            {
                /// process 3 parent code
                int pid8 = fork();
                if (pid8 == 0)
                {
                    /// process 8
                    info(BEGIN, 8, 0);

                    info(END, 8, 0);
                    exit(0);
                }
                else
                {
                    // /process 3 code

                    pthread_t th[10];

                    sem_init(&sem1, 0, 0);
                    sem_init(&sem2, 0, 0);
                    for (int i = 1; i <= 5; i++)
                    {
                        int *id = malloc(sizeof(int));
                        *id = i;
                        pthread_create(&th[i], NULL, functieP3, id);
                    }

                    for (int i = 1; i <= 5; i++)
                        pthread_join(th[i], NULL);

                    wait(NULL);
                    wait(NULL);
                    info(END, 3, 0);
                    exit(0);
                }
            }
        }
        else
        {
            wait(NULL); // dupa p2 sau p3 asteapta
            wait(NULL); // dupa cel ramas asteapta

            // destroy_them();
            info(END, 1, 0);
        }
    }
    return 0;
}