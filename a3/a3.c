#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define RESP_PIPE "RESP_PIPE_33227"
#define REQ_PIPE "REQ_PIPE_33227"

int fd1 = -1;
int fd2 = -1;
char *message = NULL;
char message_size ;
unsigned int variant = 33227;

int main()
{

    unlink(RESP_PIPE);

    if (mkfifo(RESP_PIPE, 0600) != 0)
    {
        printf("ERROR: Cannot create the response pipe\n");
        return 1;
    }

    fd1 = open(REQ_PIPE, O_RDONLY);
    if (fd1 == -1)
    {
        unlink(RESP_PIPE);
        printf("ERROR: Cannot open the request pipe\n");
        return -1;
    }

    fd2 = open(RESP_PIPE, O_WRONLY);
    if (fd2 == -1)
    {
        printf("ERROR: Cannot open the response pipe\n");
        close(fd1);
        return -1;
    }

    unsigned int connect_length = strlen("CONNECT");
    if ((write(fd2, &connect_length, 1) != -1) && (write(fd2, "CONNECT", connect_length) != -1))
    {
        printf("SUCCESS\n");
    }

 while(1)
 {
    if(read(fd1, &message_size, 1)!= 1)
    {
        printf("Error at reading size\n");
    }
    message = (char*) malloc((message_size) * sizeof(char));
        if(read(fd1, message, message_size) != message_size){
            printf("read contents error\n");
        }
        message[(int)message_size] = '\0'; 

        if(strstr(message, "PING") != NULL){
            unsigned int size_ping = 4;
            unsigned int var = 33227;
            if(write(fd2, &size_ping, 1)!= 1){
                printf("write size ping error\n");
            }
            if(write(fd2, "PING", 4)!=4){
                printf("write PING error\n");
            }
            write(fd2, &size_ping, 1);
            if(write(fd2, "PONG", 4) != 4){
                printf("write PONG error\n");
            } 

            if(write(fd2, &var, sizeof(unsigned int))!= sizeof(unsigned int)){
                printf("write var error\n");
            }
        }

        else 
        if(strcmp("EXIT", message) == 0){
            free(message);
            message = NULL;
            close(fd1);
            close(fd2);
            unlink(REQ_PIPE);
            unlink(RESP_PIPE);
            break;
        }
 }
 free(message);
 close(fd1);
 close(fd2);

return 0;

}
