#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>

int queue_id;

enum msg_type
{
    MSG_READY = 1,
    TO_JUDGE = 1000000
};

struct msg_t
{
    long addr;
    int n;
};

void judge(int n)
{
    printf("Judge: ready\n");

    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

    for(int i = 1; i <= n; ++i)
    {
        struct msg_t ready;
        
        int rcvd = msgrcv(queue_id, &ready, sizeof(int), TO_JUDGE, 0);
        
        if(rcvd < 0){
            perror("judge");
        }

        printf("Judge: runner №%d is ready\n", ready.n);
    }
    

    long start = (long) 1, finish;
    printf("Judge: runner №1 - start!\n");
    gettimeofday(&tv1, &tz1);
    msgsnd(queue_id, &start, 0, 0);

    msgrcv(queue_id, &finish, 0, (long) (n + 1), 0);
    gettimeofday(&tv2, &tz2);
    printf("Judge: runner №%d - finished\n", n);

    long int t_sec = tv2.tv_sec - tv1.tv_sec;
    long int t = tv2.tv_usec - tv1.tv_usec;

    double sec = ((double) t / 1000000) + t_sec;
    printf("Judge: time: %lf s\n", sec);
    
    printf("Judge: died\n");
    return;
}

void runner(int n)
{
    printf("Runner №%d: ready\n", n);

    struct msg_t ready_msg = {TO_JUDGE, n};

    msgsnd(queue_id, &ready_msg, sizeof(int), 0);
    
    long start_msg;
    msgrcv(queue_id, &start_msg, 0, (long)n, 0);
    printf("Runner №%d: started lap\n", n);

    long finish_msg = (long) (n + 1);
    printf("Runner №%d: finished lap\n", n);
    msgsnd(queue_id, &finish_msg, 0, 0);

    printf("Runner №%d: died\n", n);
    return;
}

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "main: Too much arguments\n");
        exit(-1);
    }

    int n = atoi(argv[1]);

    key_t key = ftok(argv[1], 1);

    queue_id = msgget(key, IPC_CREAT | 0666);
    printf("%d\n", queue_id);
    pid_t jpid = fork();

    if (jpid == 0)
    {
        judge(n);

        return 0;
    }

    for(int i = 1; i <= n; ++i)
    {
        pid_t rrpid = fork();

        if(rrpid == 0)
        {
            runner(i);
            return 0;
        }
    }

    for(int i = 0; i < n + 1; ++i)
        wait(NULL);

    
    msgctl(queue_id, IPC_RMID, 0);

    return 0;
}