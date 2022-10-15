#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>

int main(int argc, char ** argv)
{
    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

    if(argc == 1)
    {
        printf("time: %d s\n", 0);
        return 0;
    }

    if (access(argv[1], X_OK) != 0)
    {
        perror("mycp");
        exit(-1);
    }
    
    pid_t pid = fork();

    if (pid == 0)
    {
        execvp(argv[1], argv + 1);
        exit(0);
    }
    else 
    {
        gettimeofday(&tv1, &tz1);
    }

    int status;
    wait(&status);

    gettimeofday(&tv2, &tz2);

    long int t_sec = tv2.tv_sec - tv1.tv_sec;
    long int t = tv2.tv_usec - tv1.tv_usec;

    double sec = ((double) t / 1000000) + t_sec;
    printf("\ntime: %lf s\n", sec);

    return 0;
}