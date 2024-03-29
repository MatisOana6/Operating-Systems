#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2_helper.h"
#include <semaphore.h>
#include <fcntl.h>

#define NR_THREADS_7 39
#define NR_THREADS_5 5
#define NR_THREADS_3 6

pid_t pid2 = -1;
pid_t pid3 = -1;
pid_t pid4 = -1;
pid_t pid5 = -1;
pid_t pid6 = -1;
pid_t pid7 = -1;
pid_t pid8 = -1;

sem_t *sem_p5_2;
sem_t *sem_p5_3;
sem_t *sem_p3_2;
sem_t *sem_p7;
sem_t *sem_t5_4_t3_4;

pthread_mutex_t mutex_p7 = PTHREAD_MUTEX_INITIALIZER;

typedef struct
{
    int thread_number;
    int process_number;

} Thread_struct;

void *thread_function(void *arg);

void create_threads5()
{
    pthread_t threads[NR_THREADS_5];
    Thread_struct thread_vector[NR_THREADS_5];

    for (int i = 0; i < NR_THREADS_5; i++)
    {
        thread_vector[i] = (Thread_struct){
            .process_number = 5,
            .thread_number = i + 1,
        };
        pthread_create(&threads[i], NULL, thread_function, &thread_vector[i]);
    }
    for (int i = 0; i < NR_THREADS_5; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void create_threads3()
{
    pthread_t threads[NR_THREADS_3];
    Thread_struct thread_vector[NR_THREADS_3];

    for (int i = 0; i < NR_THREADS_3; i++)
    {
        thread_vector[i] = (Thread_struct){
            .process_number = 3,
            .thread_number = i + 1,
        };
        pthread_create(&threads[i], NULL, thread_function, &thread_vector[i]);
    }
    for (int i = 0; i < NR_THREADS_3; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void create_threads7()
{
    pthread_t threads[NR_THREADS_7];
    Thread_struct thread_vector[NR_THREADS_7];

    for (int i = 0; i < NR_THREADS_7; i++)
    {
        thread_vector[i] = (Thread_struct){
            .process_number = 7,
            .thread_number = i + 1,
        };

        pthread_create(&threads[i], NULL, thread_function, &thread_vector[i]);
    }
    for (int i = 0; i < NR_THREADS_7; i++)
    {
        pthread_join(threads[i], NULL);
    }
}

void *thread_function(void *arg)
{
    Thread_struct *t_struct = (Thread_struct *)arg;
    int process_id = t_struct->process_number;
    int thread_id = t_struct->thread_number;

    if ((process_id == 3 && thread_id == 2) || (process_id == 3 && thread_id == 4) || (process_id == 5 && thread_id == 4))
    {
        if (process_id == 3 && thread_id == 2)
        {
            info(BEGIN, process_id, thread_id);
            info(END, process_id, thread_id);
            sem_post(sem_p3_2);
        }
        else if (process_id == 5 && thread_id == 4)
        {
            sem_wait(sem_p3_2);
            info(BEGIN, process_id, thread_id);
            info(END, process_id, thread_id);
            sem_post(sem_t5_4_t3_4);
        }

        else if (process_id == 3 && thread_id == 4)
        {
            sem_wait(sem_t5_4_t3_4);
            info(BEGIN, process_id, thread_id);
            info(END, process_id, thread_id);
        }
    }

    else if ((process_id == 5) && (thread_id == 2 || thread_id == 3))
    {
        if (thread_id == 3)
        {
            info(BEGIN, process_id, thread_id);
            sem_post(sem_p5_3);
            sem_wait(sem_p5_2);
            info(END, process_id, thread_id);
        }
        else if (thread_id == 2)
        {
            sem_wait(sem_p5_3);
            info(BEGIN, process_id, thread_id);
            info(END, process_id, thread_id);
            sem_post(sem_p5_2);
        }
    }

    else if (process_id == 7)
    {
        sem_wait(sem_p7);
        info(BEGIN, process_id, thread_id);
        pthread_mutex_lock(&mutex_p7);
        info(END, process_id, thread_id);
        sem_post(sem_p7);
        pthread_mutex_unlock(&mutex_p7);
    }
    else
    {
        info(BEGIN, process_id, thread_id);
        info(END, process_id, thread_id);
    }

    return NULL;
}

int main(int argc, char **argv)
{
    sem_unlink("sem_p5_2");
    sem_unlink("sem_p5_3");
    sem_unlink("sem_p3_2");
    sem_unlink("sem_p7");
    sem_unlink("sem_t5_4_t3_4");

    sem_p5_2 = sem_open("sem_p5_2", O_CREAT, 0644, 0);
    sem_p5_3 = sem_open("sem_p5_3", O_CREAT, 0644, 0);
    sem_p3_2 = sem_open("sem_p3_2", O_CREAT, 0644, 0);
    sem_t5_4_t3_4 = sem_open("sem_t5_4_t3_4", O_CREAT, 0644, 0);
    sem_p7 = sem_open("sem_p7", O_CREAT, 0644, 5);

    init();

    sem_init(sem_p3_2, 1, 0);
    sem_init(sem_t5_4_t3_4, 1, 0);
    sem_init(sem_p7, 1, 5);
    sem_init(sem_p5_2, 1, 0);
    sem_init(sem_p5_3, 1, 0);

    info(BEGIN, 1, 0);
    pid2 = fork();
    if (pid2 == -1)
    {
        perror("Unsuccessful creation of child process");
        return 1;
    }
    else if (pid2 == 0)
    {
        info(BEGIN, 2, 0);
        create_threads3();
        pid3 = fork();
        if (pid3 == -1)
        {
            perror("Unsuccessful creation of child process");
            return 1;
        }
        else if (pid3 == 0)
        {
            info(BEGIN, 3, 0);
            info(END, 3, 0);
            return 0;
        }
        else
        {

            pid4 = fork();
            if (pid4 == -1)
            {
                perror("Unsuccessful creation of child process");
                return 1;
            }
            else if (pid4 == 0)
            {
                info(BEGIN, 4, 0);
                create_threads7();
                pid7 = fork();
                if (pid7 == -1)
                {
                    perror("Unsuccessful creation of child process");
                    return 1;
                }
                else if (pid7 == 0)
                {
                    info(BEGIN, 7, 0);
                    info(END, 7, 0);
                    return 0;
                }
                else
                {
                    waitpid(pid7, NULL, 0);
                    info(END, 4, 0);
                    return 0;
                }
            }
            else
            {
                waitpid(pid4, NULL, 0);
                info(END, 2, 0);
                return 0;
            }
        }
    }
    else
    {
        create_threads5();
        pid5 = fork();
        if (pid5 == -1)
        {
            perror("Unsuccessful creation of child process");
            return 1;
        }
        else if (pid5 == 0)
        {
            info(BEGIN, 5, 0);

            pid6 = fork();
            if (pid6 == -1)
            {
                perror("Unsuccessful creation of child process");
                return 1;
            }
            else if (pid6 == 0)
            {
                info(BEGIN, 6, 0);

                pid8 = fork();
                if (pid8 == -1)
                {
                    perror("Unsuccessful creation of child process");
                    return 1;
                }
                else if (pid8 == 0)
                {
                    info(BEGIN, 8, 0);
                    info(END, 8, 0);
                    return 0;
                }
                else
                {
                    waitpid(pid8, NULL, 0);
                    info(END, 6, 0);
                    return 0;
                }
            }
            else
            {
                waitpid(pid6, NULL, 0);
                info(END, 5, 0);
                return 0;
            }
        }
        else
        {
            waitpid(pid3, NULL, 0);
            waitpid(pid2, NULL, 0);
            waitpid(pid5, NULL, 0);
            info(END, 1, 0);
            return 0;
        }
    }
    sem_close(sem_p5_3);
    sem_close(sem_p5_2);
    sem_close(sem_p7);
    sem_close(sem_t5_4_t3_4);
    sem_close(sem_p3_2);

    sem_unlink("sem_p7");
    sem_unlink("sem_p5_2");
    sem_unlink("sem_p5_3");
    sem_unlink("sem_p3_2");
    sem_unlink("sem_t5_4_t3_4");

    return 0;
}
