#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "a2_helper.h"

void *thread_function(void *arg)
{
    info(BEGIN,5,1);
    info(BEGIN,5,3);
    info(BEGIN,5,2);
    info(BEGIN,5,4);
    info(BEGIN,5,5);

     info(END,5,2);
     info(END,5,3);
     info(END,5,4);
     info(END,5,5);
     info(END,5,1);

     return NULL;
}


int main(int argc, char **argv)
{
    pid_t pid2, pid3, pid4, pid5, pid6, pid7, pid8;
    init();
    info(BEGIN, 1, 0);

    pid2 = fork();
    if (pid2 == -1)
    {
        perror("Nu se poate crea proces");
        return 1;
    }
    else if (pid2 == 0)
    {
        info(BEGIN, 2, 0);
        pid3 = fork();
        if (pid3 == -1)
        {
            perror("Nu se poate crea proces");
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
            waitpid(pid3, NULL, 0);

            pid4 = fork();
            if (pid4 == -1)
            {
                perror("Nu se poate crea proces");
                return 1;
            }
            else if (pid4 == 0)
            {
                info(BEGIN, 4, 0);
                pid7 = fork();
                if (pid7 == -1)
                {
                    perror("Nu se poate crea proces");
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
        waitpid(pid2, NULL, 0);

        pid5 = fork();
        if (pid5 == -1)
        {
            perror("Nu se poate crea proces");
            return 1;
        }
        else if (pid5 == 0)
        {
            info(BEGIN, 5, 0);
            pthread_t tid;
            pthread_create(&tid, NULL, thread_function, NULL);
            pid6 = fork();
            if (pid6 == -1)
            {
                perror("Nu se poate crea proces");
                return 1;
            }
            else if (pid6 == 0)
            {
                info(BEGIN, 6, 0);

                pid8 = fork();
                if (pid8 == -1)
                {
                    perror("Nu se poate crea proces");
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
                pthread_cancel(tid);
                info(END, 5, 0);
                return 0;
            }
        }
        else
        {
            waitpid(pid5, NULL, 0);
            info(END, 1, 0);
            return 0;
        }
    }

    return 0;
}
