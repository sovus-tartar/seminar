#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <limits.h>
#include <stdlib.h>
#include <grp.h>
#include <getopt.h>

void print_groups(char * user, gid_t group)
{
    int ngroups;
    ngroups = sysconf(_SC_NGROUPS_MAX);

    gid_t *list = malloc(sizeof(gid_t)*ngroups);

    getgrouplist(user, group, list, &ngroups);

    printf("groups=");
    for(int i = 0; i < ngroups; ++i)
    {
        struct group *gr_info = getgrgid(list[i]);

        printf("%u(%s)", gr_info->gr_gid, gr_info->gr_name);

        if (i != (ngroups - 1))
            putchar(',');        
    }

    free(list);
}

void print_usr_inf()
{
    uid_t uid = geteuid();
    struct passwd *user_info = getpwuid(uid);

    gid_t gid = getgid();
    struct group *group_info = getgrgid(gid);

    printf("uid=%u(%s) gid=%u(%s) ", uid, user_info->pw_name, gid, group_info->gr_name);

    print_groups(user_info->pw_name, gid);
}

void print_other_usr_inf(const char * name)
{   
    struct passwd *user_info;
    struct group *group_info;
    gid_t gid;

    if (name == NULL)
    {
        uid_t uid = geteuid();
        user_info = getpwuid(uid);

        gid = getgid();
        group_info = getgrgid(gid);
    }
    else 
    {
        user_info = getpwnam(name);

        gid = user_info->pw_gid;
        group_info = getgrgid(gid);
    }

    

    printf("uid=%u(%s) gid=%u(%s) ", user_info->pw_uid, user_info->pw_name, gid, group_info->gr_name);
    
    print_groups(user_info->pw_name, gid);
}

int main(int argc, char * argv[])
{


    switch(argc)
    {
        case 1:
            print_usr_inf();
            break;
        case 2:
            print_other_usr_inf(argv[1]);
            break;
        default:
            fprintf(stderr, "myid: main: wrong number of arguments");

    }

    putchar('\n');
}
