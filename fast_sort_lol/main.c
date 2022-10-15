#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


int main(int argc, char ** argv)
{
    for(int i = 1; i < argc; ++i)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            int num = atoi(argv[i]);
            usleep((useconds_t) num * 10000);
            printf("%d ", num);

            exit(0);
        } 

    }

    for(int i = 1; i < argc; ++i) {
        int status;
        wait(&status);
    }


    putchar('\n');

    return 0;
}