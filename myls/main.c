#include <stdio.h>
#include <getopt.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>

int args[7];

//TODO: -L

enum ARG_LIST
{
    ARG_R = 0, //done
    ARG_l = 1,
    ARG_n = 2, //
    ARG_d = 3, //done
    ARG_a = 4, //done
    ARG_i = 5, //done
    ARG_L = 6 //?
};

void print_l()
{
    
}

void print_dir(DIR *curr_dir, char * curr_path)
{

    struct dirent *curr_st;

    while (curr_st = readdir(curr_dir))
    {
        if(args[ARG_d])
        {
            if (!strcmp(curr_st->d_name, "."))
            {
                if (args[ARG_i])
                    printf("%ld ", curr_st->d_ino);
                
                printf("%s\n", curr_path);
            }

            break;
        }

        if (((curr_st->d_name[0] != '.') || (args[ARG_a])) || (args[ARG_d] && (!strcmp(curr_st->d_name, ".")))) 
        {
            if (args[ARG_i])
                printf("%ld ", curr_st->d_ino);

            printf("%s\n", curr_st->d_name);
        }
    }
}

void recursive_run(char *path_arr, int curr_pos)
{
    struct dirent *curr_st;
    DIR * curr_dir;

    curr_dir = opendir(path_arr);
    printf("%s:\n", path_arr);
    print_dir(curr_dir, path_arr);
    rewinddir(curr_dir);
    putchar('\n');

    while (curr_st = readdir(curr_dir))
    {
        if((curr_st->d_type == DT_DIR) && !(curr_st->d_name[0] == '.'))
        {
            strcpy(path_arr + curr_pos, "/");
            strcpy(path_arr + curr_pos + 1, curr_st->d_name);
            recursive_run(path_arr, curr_pos + 1 + strlen(curr_st->d_name));
            path_arr[curr_pos] = '\0';
        }
    }

    closedir(curr_dir);
}

void run(char **path_arr, int num)
{
    DIR *curr_dir;

    if (*path_arr == NULL)
    {
        if ((args[ARG_R] == 0) || (args[ARG_d]))
        {
            curr_dir = opendir(".");
            print_dir(curr_dir, ".");
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
                recursive_run(path_arr[i], strlen(path));
            }
            else
            {
                if (num != 1)
                {
                    printf("%s:\n", path_arr[i]);
                }

                curr_dir = opendir(path_arr[i]);
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