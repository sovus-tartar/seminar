#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define kb 1024

int set_i = 0;
int set_f = 0;
int set_v = 0;

void clean_buffer()
{
    char c;
    while ((c = getchar()) != '\n')
        ;
}

int copy(int fd_in, int fd_out)
{
    char buf[4 * kb] = "";
    ssize_t char_read = 0;

    while ((char_read = read(fd_in, buf, 4 * kb)) != 0)
    {
        if (char_read < 0)
        {
            perror("cp");
            exit(-1);
        }

        ssize_t succ_write = 0;

        while ((succ_write = write(fd_out, buf, char_read)) < char_read)
        {
            char_read -= succ_write;
            if (succ_write < 0)
            {
                perror("cp");
                exit(-1);
            }
        }
    }

    return 0;
}

int copy_file_to_file(char *file1, char *file2)
{
    int fd_1 = -1, fd_2 = -1;

    if ((set_i > 0) && (!access(file2, F_OK)))
    {
        if ((access(file2, R_OK)) || (access(file2, W_OK)))
        {
            if (set_i)
            {
                printf("cp: replace '%s', overriding mode 0555 (r-xr-xr-x)?", file2);
                if (getchar() != 'y')
                {
                    return 0;
                }

                clean_buffer();
            }

            if (unlink(file2) == -1)
            {
                perror("cp");
                exit(-1);
            }

            if (set_v)
            {
                printf("removed '%s'\n", file2);
            }
        }
        else
        {
            printf("cp: overwrite '%s'?", file2);
            if (getchar() != 'y')
                return 0;
            clean_buffer();
        }
    }

    fd_1 = open(file1, O_RDONLY);

    if (fd_1 == -1)
    {
        perror("cp");
        exit(-1);
    }

    fd_2 = open(file2, O_CREAT | O_RDWR | O_TRUNC, S_IRWXU);

    if (fd_2 == -1)
    {
        perror("cp");
        exit(-1);
    }

    copy(fd_1, fd_2);

    close(fd_1);
    close(fd_2);

    if (set_v > 0)
        printf("'%s' -> '%s'\n", file1, file2);

    return 0;
}

char *create_filepath(char *dir, char *file)
{
    char *str = malloc(sizeof(char) * (strlen(dir) + strlen(file) + 2));

    str = strcpy(str, dir);

    strcat(str, "/");
    strcat(str, file);

    return str;
}

char *get_filename(char *path)
{
    int n = strlen(path);
    char *ptr = path;
    for (int i = 0; i < n; ++i)
        if (path[i] == '/')
            ptr = path + i + 1;

    return ptr;
}

int copy_files_to_dir(char *files[], int n, char *dir)
{
    for (int i = 0; i < n; ++i)
    {
        char *filepath = files[i];

        struct stat file_info = {0};

        if (stat(filepath, &file_info) == -1)
        {
            errno = ENOENT;
            return -1;
        }

        char *new_path = create_filepath(dir, files[i]);

        copy_file_to_file(files[i], new_path);

        free(new_path);
    }
}

int is_folder(char *path)
{
    struct stat buf;

    if (stat(path, &buf) == -1)
        return 0;

    if ((buf.st_mode & S_IFMT) == S_IFDIR)
        return 1;

    return 0;
}

int main(int argc, char *argv[])
{
    int opt = 0;
    int read = 0;
    int fd_out = -1;
    char *path_out = NULL;
    int is_dir = -1;

    while ((opt = getopt(argc, argv, "vfi")) != -1)
    {
        switch (opt)
        {
        case 'v':
            set_v = 1;
            read += 1;
            break;
        case 'f':
            set_f = 1;
            read += 1;
            break;
        case 'i':
            set_i = 1;
            read += 1;
            break;
        default:
            exit(-1);
        }
    }

    path_out = argv[argc - 1];

    is_dir = is_folder(path_out);

    if (is_dir == -1)
    {
        perror("cp");
        exit(-1);
    }

    if (is_dir == 0)
    {
        if (((argc - 1) - (read + 1)) > 1)
        {
            errno = ENOTDIR;
            perror("cp");
            exit(-1);
        }

        if (((argc - 1) - (read + 1)) == 0)
        {
            fprintf(stderr, "cp: missing destination file operand for '%s'\n", argv[argc - 1]);
            exit(-1);
        }

        copy_file_to_file(argv[argc - 2], argv[argc - 1]);
    }

    if (is_dir == 1)
    {
        copy_files_to_dir(argv + read + 1, argc - read - 2, path_out);
    }

    return 0;
}
