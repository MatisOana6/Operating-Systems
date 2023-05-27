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
#define PING "PING"
#define PONG "PONG"

int fd1 = -1;
int fd2 = -1;
char *message = NULL;
unsigned int message_size;
unsigned int variant = 33227;
unsigned int connect_nr = strlen("CONNECT");
unsigned int crt_nr = strlen("CREATE_SHM");
unsigned int error_nr = strlen("ERROR");
unsigned int success_nr = strlen("SUCCESS");
unsigned int nr;
int shmFd = -1;
volatile char *map_file = NULL;
int map_file_size;
volatile char *sharedChar = NULL;
/**char* read_pipe(int fd)
{
    char* string = calloc(255,1);
    int nr = 0;
    read(fd, &string[nr++],1);
    while(string[nr-1]!= '$')
    {
        read(fd, &string[nr++],1);
    }
    return string;
}*/

int main()
{
    if (access(RESP_PIPE, 0) == 0)
    {
        unlink(RESP_PIPE);
    }

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

    unsigned int connect_length = strlen("CONNECT$");
    if (write(fd2, "CONNECT$", connect_length) != -1)
    {
        printf("SUCCESS\n");
    }
    while (1)
    {

        char command[255] = {0};
        int nr = 0;

        read(fd1, &command[nr], 1);

        nr++;
        while (command[nr - 1] != '$')
        {
            read(fd1, &command[nr], 1);
            nr++;
        }

        if (strcmp(command, "PING$") == 0)
        {
            // Send the ping response message
            write(fd2, "PING$", 5);
            write(fd2, &variant, sizeof(unsigned int));
            write(fd2, "PONG$", 5);
        }
        else if (strcmp(command, "CREATE_SHM$") == 0)
        {
            int nr = 0;
            read(fd1, &nr, 4);

            shmFd = shm_open("/Nj2X1X", O_CREAT | O_RDWR, 0644);
            if (shmFd < 0)
            {
                write(fd2, "CREATE_SHM$ERROR$", 17);
            }
            ftruncate(shmFd, nr * sizeof(char));
            write(fd2, "CREATE_SHM$SUCCESS$", 19);
        }
        else if (strcmp(command, "WRITE_TO_SHM$") == 0)
        {
            unsigned int offset;
            unsigned int value;
            read(fd1, &offset, 4);
            read(fd1, &value, 4);
            printf("%d %d\n", offset, value);
            if ((offset < 0 || offset > 3734652) || (offset + 4 > 3734652))
            {
                write(fd2, "WRITE_TO_SHM$ERROR$", 19);
            }
            else
            {

                sharedChar = (volatile char *)mmap(0, 3734652, PROT_READ | PROT_WRITE,
                                                   MAP_SHARED, shmFd, 0);

                memcpy((char *)sharedChar + offset, &value, 4);
                munmap((void *)sharedChar, sizeof(char));

                write(fd2, "WRITE_TO_SHM$SUCCESS$", 21);
            }
        }
        else if (strcmp(command, "MAP_FILE$") == 0)
        {

            char file_name[255] = {0};
            int nr = 0;

            read(fd1, &file_name[nr], 1);

            nr++;
            while (file_name[nr - 1] != '$')
            {
                read(fd1, &file_name[nr], 1);
                nr++;
            }
            file_name[--nr] = 0;
            //  printf("%s\n", file_name);

            int fd_new = -1;

            fd_new = open(file_name, O_RDONLY);
            map_file_size = lseek(fd_new, 0, SEEK_END);

            map_file = (volatile char *)mmap(0, map_file_size, PROT_READ,
                                             MAP_SHARED, fd_new, 0);

            if (map_file == (void *)-1)
            {
                munmap((void *)map_file, map_file_size);
                close(fd_new);
                write(fd2, "MAP_FILE$ERROR$", 15);
            }
            else
            {
                close(fd_new);
                write(fd2, "MAP_FILE$SUCCESS$", 17);
            }
        }
        else if (strcmp(command, "READ_FROM_FILE_OFFSET$") == 0)
        {
            unsigned int offset;
            unsigned int nr_bytes;

            read(fd1, &offset, 4);
            read(fd1, &nr_bytes, 4);

            sharedChar = (volatile char *)mmap(0, 3734652, PROT_READ | PROT_WRITE,
                                               MAP_SHARED, shmFd, 0);
            if (sharedChar != NULL && map_file != NULL && offset + nr_bytes <= map_file_size)
            {
                memcpy((void *)sharedChar, (void *)map_file + offset, nr_bytes);
                write(fd2, "READ_FROM_FILE_OFFSET$SUCCESS$", 30);
            }

            else
            {
                write(fd2, "READ_FROM_FILE_OFFSET$ERROR$", 28);
            }
        }
        else if (strcmp(command, "READ_FROM_FILE_SECTION$") == 0)
        {
            sharedChar = (volatile char *)mmap(0, 3734652, PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0);

            unsigned int offset;
            unsigned int nr_bytes;
            unsigned int no_section;
            read(fd1, &no_section, sizeof(unsigned int));
            read(fd1, &offset, sizeof(unsigned int));
            read(fd1, &nr_bytes, sizeof(unsigned int));

            char magic;
            char version[5];
            unsigned int section_size = 0;
            unsigned int section_offset = 0;
            unsigned int no_of_sections = 0;

            memcpy((void *)&magic, (void *)map_file, 1);
            memcpy((void *)version, (void *)map_file, 4);
            version[4] = '\0';
            memcpy((void *)&no_of_sections, (void *)map_file + 7, 1);

            if ((magic == 'v') && (atoi(version) >= 56 && atoi(version) <= 96) && (no_of_sections >= 2 && no_of_sections <= 16) && (no_section >= 1 && no_section <= no_of_sections))
            {
                memcpy((void *)&section_offset, (void *)map_file + (8 + (no_of_sections * (8 + 1 + 4 + 4)) + ((no_section - 1) * (8 + 1 + 4 + 4))), sizeof(unsigned int));
                memcpy((void *)&section_size, (void *)map_file + (8 + (no_of_sections * (8 + 1 + 4 + 4)) + ((no_section - 1) * (8 + 1 + 4 + 4))) + sizeof(unsigned int), sizeof(unsigned int));

                if (offset + nr_bytes <= section_size)
                {
                    memcpy((void *)sharedChar, (void *)(map_file + section_offset + offset), nr_bytes);
                    write(fd2, "READ_FROM_FILE_SECTION$SUCCESS$", 31);
                }
                else
                {
                    write(fd2, "READ_FROM_FILE_SECTION$ERROR$", 29);
                }
            }
            else
            {
                write(fd2, "READ_FROM_FILE_SECTION$ERROR$", 29);
            }
        }
        else if (strcmp(command, "READ_FROM_LOGICAL_SPACE_OFFSET$") == 0)
        {
            unsigned int logical_offset;
            unsigned int nr_bytes;
            read(fd1, &logical_offset, 4);
            read(fd1, &nr_bytes, 4);
            char magic;
            char *version = (char *)malloc(sizeof(char) * 5);

            unsigned int no_of_sections = 0;

            memcpy((void *)&magic, (void *)map_file, 1);
            memcpy((void *)&version, (void *)map_file, 4);
            version[4] = '\0';
            memcpy((void *)&no_of_sections, (void *)map_file + 7, 1);
        }

        else if (strcmp(command, "EXIT$") == 0)
        {
            // Close the pipes and free the memory
            close(fd2);
            close(fd1);
            close(shmFd);
            munmap((void *)map_file, map_file_size);
            shm_unlink("/Nj2X1X");

            // Delete the response pipe
            unlink(RESP_PIPE);
            break; // Terminate the loop
        }
    }

    return 0;
}
