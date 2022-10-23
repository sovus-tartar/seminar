#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <pwd.h>
#include <grp.h>
#include <unistd.h>
#include <time.h>

int args[7];

const int default_block_size = 512;
int block_size = 1024;

// TODO: -L?

enum ARG_LIST
{
    ARG_R = 0, // done
    ARG_l = 1, //done
    ARG_n = 2, // done
    ARG_d = 3, // done
    ARG_a = 4, // done
    ARG_i = 5, // done
};

void strmode(mode_t mode, char *buf)
{
    const char chars[] = "rwxrwxrwx";
    for (size_t i = 0; i < 9; i++)
    {
        buf[i] = (mode & (1 << (8 - i))) ? chars[i] : '-';
    }
    buf[9] = '\0';
}

void print_ln(char *path)
{
    struct stat file_stat;
    char mode_str[10];
    int res = lstat(path, &file_stat);

    if (res)
    {
        perror("print_ln");
        printf("????????? ");
        return;
    }

    strmode(file_stat.st_mode, mode_str);

    if (S_ISDIR(file_stat.st_mode))
        putchar('d');
    else if (S_ISSOCK(file_stat.st_mode))
        putchar('s');
    else if (S_ISLNK(file_stat.st_mode))
        putchar('l');
    else if (S_ISBLK(file_stat.st_mode))
        putchar('b');
    else if (S_ISCHR(file_stat.st_mode))
        putchar('c');
    else
        putchar('-');

    printf("%s ", mode_str);

    printf("%ld ", file_stat.st_nlink);
    if (args[ARG_n])
        printf("%d %d ", file_stat.st_uid, file_stat.st_gid);
    else
    {
        struct passwd *user_info = getpwuid(file_stat.st_uid);
        struct group *group_info = getgrgid(file_stat.st_gid);

        printf("%s %s ", user_info->pw_name, group_info->gr_name);
    }

    printf("%ld ", file_stat.st_size);

    char timebuff[255];
    struct tm time_;
    localtime_r(&file_stat.st_mtim.tv_sec, &time_);
    strftime(timebuff, sizeof(timebuff), "%b %d %H:%M ", &time_);
    printf("%s ", timebuff);
}

void print_blocks(DIR *curr_dir, char *curr_path)
{
    struct dirent *curr_st;

    int blocks_used = 0;
    while (curr_st = readdir(curr_dir))
    {
        int path_len = strlen(curr_path);

        strcpy(curr_path + path_len, "/");
        strcpy(curr_path + path_len + 1, curr_st->d_name);

        struct stat st;
        stat(curr_path, &st);
        blocks_used += st.st_blocks;

        curr_path[path_len] = '\0';
    }

    printf("total %d\n", blocks_used * default_block_size / block_size);

    rewinddir(curr_dir);
}

void print_dir(DIR *curr_dir, char *curr_path)
{

    struct dirent *curr_st;

    if ((args[ARG_l] || args[ARG_n]))
    {
        print_blocks(curr_dir, curr_path);
    }

    while (curr_st = readdir(curr_dir))
    {
        if (((curr_st->d_name[0] != '.') || (args[ARG_a]) || (args[ARG_d])))
        {

            if (args[ARG_i])
                printf("%ld ", curr_st->d_ino);

            if (args[ARG_n] || args[ARG_l])
            {

                int path_len = strlen(curr_path);

                strcpy(curr_path + path_len, "/");
                strcpy(curr_path + path_len + 1, curr_st->d_name);

                print_ln(curr_path);

                curr_path[path_len] = '\0';
            }

            printf("%s\n", curr_st->d_name);

            if (args[ARG_d])
                break;
        }
    }
}

void recursive_run(char *path_arr, int curr_pos)
{
    struct dirent *curr_st;
    DIR *curr_dir;

    curr_dir = opendir(path_arr);

    if (!curr_dir)
    {
        perror("recursive_run");
        return;
    }

    printf("%s:\n", path_arr);
    print_dir(curr_dir, path_arr);
    rewinddir(curr_dir);
    putchar('\n');

    while (curr_st = readdir(curr_dir))
    {
        if ((curr_st->d_type == DT_DIR) && !(curr_st->d_name[0] == '.'))
        {
            strcpy(path_arr + curr_pos, "/");
            strcpy(path_arr + curr_pos + 1, curr_st->d_name);
            recursive_run(path_arr, curr_pos + 1 + strlen(curr_st->d_name));
            path_arr[curr_pos] = '\0';
        }
    }

    closedir(curr_dir);
}

void remove_slash(char * ptr)
{
    int size = strlen(ptr);
    if (ptr[size - 1] == '/')
        ptr[size - 1] = 0;
}

void run(char **path_arr, int num)
{
    DIR *curr_dir;

    if (*path_arr == NULL)
    {
        if ((args[ARG_R] == 0) || (args[ARG_d]))
        {
            curr_dir = opendir(".");

            if (!curr_dir)
            {
                perror("run");
                return;
            }
            char path[PATH_MAX] = ".";
            print_dir(curr_dir, path);
            closedir(curr_dir);
        }
        else
        {
            char path[PATH_MAX] = ".";
            recursive_run(path, strlen(path));
        }
    }
    else
    {
        for (int i = 0; i < num; ++i)
        {
            if ((args[ARG_R]) && (!args[ARG_d]))
            {
                char path[PATH_MAX];
                strcpy(path, path_arr[i]);
                remove_slash(path);
                recursive_run(path_arr[i], strlen(path));
            }
            else
            {
                if (num != 1)
                {
                    printf("%s:\n", path_arr[i]);
                }

                curr_dir = opendir(path_arr[i]);

                if (!curr_dir)
                {
                    perror("recursive_run");
                    exit(-1);
                }

                print_dir(curr_dir, path_arr[i]);
                closedir(curr_dir);

                if (i != (num - 1))
                    printf("\n");
            }
        }
    }
}

int main(int argc, char **argv)
{
    int opt = 0;

    char **path_arr = NULL;
    int num_files = 0;

    while ((opt = getopt(argc, argv, "ldaRin")) != -1)
    {
        switch (opt)
        {
        case 'R':
            args[ARG_R] = 1;
            break;
        case 'l':
            args[ARG_l] = 1;
            break;

        case 'n':
            args[ARG_n] = 1;
            break;

        case 'd':
            args[ARG_d] = 1;
            break;
        case 'a':
            args[ARG_a] = 1;
            break;
        case 'i':
            args[ARG_i] = 1;
            break;

        default:
            perror("myls");
        }
    }

    path_arr = argv + optind;
    num_files = argc - optind;

    run(path_arr, num_files);
}