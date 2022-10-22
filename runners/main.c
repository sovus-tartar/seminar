#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/wait.h>

int queue_id;

enum msg_type
{
    MSG_READY = 1
};


void judge(int n)
{
    printf("Judge: ready\n");

    for(int i = 1; i <= n; ++i)
    {
        struct msg_t ready;
        msgrcv(queue_id, &ready, 0, i, 0);
        printf("Judge: runner №%d is ready\n", ready.n);
    }
    
    struct msg_t start, finish;
    start = {}
    msgsnd()
}

void runner(int n)
{
    printf("Runner №%d: ready\n", n);

    long ready_msg = (long) n;

    msgsnd(queue_id, &ready_msg, 0, 0);

    

    printf("Runner №%d: died\n", n);
}

int main(int argc, char ** argv)
{
    if(argc != 2)
    {
        fprintf(stderr, "main: Too much arguments\n");
        exit(-1);
    }

    int n = atoi(argv[1]);

    key_t key = ftok(argv[0], 1);

    queue_id = msgget(key, IPC_CREAT);

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

    wait(NULL);



    return 0;
}