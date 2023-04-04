#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

#define MAX_SIZE 2048

typedef struct Header_section
{
    char section_name[8];
    unsigned char section_type;
    int section_offset;
    int section_size;
} Header_section;

typedef struct Header
{
    char magic;
    short int header_size;
    int version;
    unsigned char no_of_sections;
    Header_section *header_sections;

} Header;

int compare(const char *s1, const char *s2)
{
    int j = strlen(s1) - 1;

    for (int i = strlen(s2) - 1; i >= 0; i--, j--)
    {
        if (s1[j] != s2[i])

            return 0;
    }
    return 1;
}

int simpleList(const char *path, off_t st_size, const char *nameEnd)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char filePath[MAX_SIZE];
    struct stat statbuf;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("ERROR");
        perror("invalid directory path");
        return -1;
    }

    printf("SUCCESS\n");

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(filePath, MAX_SIZE, "%s/%s", path, entry->d_name);
            if (lstat(filePath, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {

                    if (strcmp(nameEnd, "") == 0)
                    {
                        if (st_size != -1 && statbuf.st_size < st_size)
                            printf("%s\n", filePath);
                        else if (st_size == -1)
                            printf("%s\n", filePath);
                    }
                    else
                    {
                        if (compare(entry->d_name, nameEnd))
                        {
                            if (st_size != -1 && statbuf.st_size < st_size)
                                printf("%s\n", filePath);
                            else if (st_size == -1)
                                printf("%s\n", filePath);
                        }
                    }
                }
            }
        }
    }

    closedir(dir);
    return 0;
}

void listRecursive(const char *path, int recursiv, off_t st_size, const char *nameEnd)
{
    DIR *dir = NULL;
    struct dirent *entry = NULL;
    char f_path[MAX_SIZE];
    struct stat statbuf;

    dir = opendir(path);
    if (dir == NULL)
    {
        perror("ERROR");
        return;
    }
    if (recursiv == 1)
    {
        printf("SUCCESS\n");
        recursiv--;
    }
    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0)
        {
            snprintf(f_path, MAX_SIZE, "%s/%s", path, entry->d_name);
            if (lstat(f_path, &statbuf) == 0)
            {
                if (S_ISREG(statbuf.st_mode))
                {
                    if (strcmp(nameEnd, "") == 0)
                    {
                        if (st_size != -1 && statbuf.st_size < st_size)
                            printf("%s\n", f_path);
                        else if (st_size == -1)
                            printf("%s\n", f_path);
                    }
                    else
                    {
                        if (compare(entry->d_name, nameEnd))
                        {
                            if (st_size != -1 && statbuf.st_size < st_size)
                                printf("%s\n", f_path);
                            else if (st_size == -1)
                                printf("%s\n", f_path);
                        }
                    }
                }

                else if (S_ISDIR(statbuf.st_mode))
                {
                    if ((st_size == -1 && strcmp(nameEnd, "") == 0) || (strcmp(nameEnd, "") != 0 && compare(entry->d_name, nameEnd)))
                        printf("%s\n", f_path);
                    listRecursive(f_path, recursiv, st_size, nameEnd);
                }
            }
        }
    }
    closedir(dir);
}

void parse(const char *path)
{
    Header h;

    int fd = open(path, O_RDONLY);
    read(fd, &h.magic, 1);
    read(fd, &h.header_size, 2);
    read(fd, &h.version, 4);
    read(fd, &h.no_of_sections, 1);

    if (h.magic != 'v')
    {
        printf("ERROR\nwrong magic\n");
        close(fd);
        return;
    }

    if (h.version < 56 || h.version > 96)
    {
        printf("ERROR\nwrong version\n");
        close(fd);
        return;
    }

    if (h.no_of_sections < 2 || h.no_of_sections > 16)
    {
        printf("ERROR\nwrong sect_nr\n");
        close(fd);
        return;
    }

    h.header_sections = (Header_section *)malloc(h.no_of_sections * sizeof(Header_section));

    for (int i = 0; i < h.no_of_sections; i++)
    {

        read(fd, &h.header_sections[i].section_name, 8);
        read(fd, &h.header_sections[i].section_type, 1);

        if (h.header_sections[i].section_type != 45 && h.header_sections[i].section_type != 37 && h.header_sections[i].section_type != 92 && h.header_sections[i].section_type != 79)
        {
            printf("ERROR\nwrong sect_types\n");
            free(h.header_sections);
            return;
        }

        read(fd, &h.header_sections[i].section_offset, 4);
        read(fd, &h.header_sections[i].section_size, 4);
    }

    printf("SUCCESS\n");
    printf("version=%d\n", h.version);
    printf("nr_sections=%d\n", h.no_of_sections);

    for (int i = 0; i < h.no_of_sections; i++)
    {
        printf("section%d: ", i + 1);
        for (int j = 0; j < 8; j++)
        {
            if (h.header_sections[i].section_name[j] == '\0')
                break;
            printf("%c", h.header_sections[i].section_name[j]);
        }
        printf(" %d %d\n", h.header_sections[i].section_type, h.header_sections[i].section_size);
    }
    free(h.header_sections);
}

int main(int argc, char **argv)
{

    char path[1000] = "";
    int parse_var = 0;
    int recursive = 0;
    int list = 0;
    char ends_with[MAX_SIZE] = "";
    int smaller = -1;

    if (argc >= 2)
    {
        if (strstr(argv[1], "variant"))
        {

            printf("33227\n");
        }
        for (int i = 1; i < argc; i++)
        {
            if (strncmp(argv[i], "path=", 5) == 0)
                for (int j = 5; j < strlen(argv[i]); j++)
                {
                    path[j - 5] = argv[i][j];
                }
            else if (strcmp(argv[i], "parse") == 0)
            {
                parse_var = 1;
            }
            else if (strcmp(argv[i], "recursive") == 0)
            {
                recursive = 1;
            }
            else if (strcmp(argv[i], "list") == 0)
            {
                list = 1;
            }
            else if (strncmp(argv[i], "name_ends_with=", 15) == 0)
            {
                for (int j = 15; j < strlen(argv[i]); j++)
                {
                    ends_with[j - 15] = argv[i][j];
                }
            }
            else if (strncmp(argv[i], "size_smaller=", 13) == 0)
            {
                char var[100] = "";
                for (int j = 13; j < strlen(argv[i]); j++)
                {

                    var[j - 13] = argv[i][j];
                }

                smaller = atoi(var);
            }
        }
        if (list == 1 && recursive == 0)
        {

            simpleList(path, smaller, ends_with);
        }
        else if (list == 1 && recursive == 1)
        {

            listRecursive(path, 1, smaller, ends_with);
        }
        else if (parse_var == 1)
        {
            parse(path);
        }
        return 0;
    }
}
