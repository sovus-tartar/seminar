// my echo program
// TODO: -n

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <sys/types.h>
#include <unistd.h>

int is_notnewline(char *arg) // 1 if -n
{
    return !strcmp(arg, "-n");
}

int arg_count(int argc, char *argv[])
{
    int count = 0;

    for(int i = 1; i < argc; i++)
        count += is_notnewline(argv[i]);

    return count;
}

int main(int argc, char *argv[])
{
    int state;
    assert(argc);

    printf("pid: %d\nppid: %d\n", getpid(), getppid());


    switch (argc)
    {
    case 1:
        fputs("\n", stdout);
        break;
        
    case 2:
        if (!is_notnewline(argv[1]))
        {
            fputs(argv[1], stdout);
            fputs("\n", stdout);
        }
        break;

    default:
        state = is_notnewline(argv[1]);
        int i = arg_count(argc, argv) + 1;

        for (; i < argc; i++)
        {
            fputs(argv[i], stdout);
            
            if (i < argc - 1)
                fputs(" ", stdout);

        }

        if (!state)
            fputs("\n", stdout);

        break;
    }

    return 0;
}
