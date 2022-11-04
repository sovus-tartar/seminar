#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sem.h>

int v(int sem_id) // unlock
{
    struct sembuf set1 = {0, 1, 0};
    semop(sem_id, &set1, 1);
    return 1;
}

int p(int sem_id) // lock
{
    struct sembuf set0 = {0, -1, 0};
    semop(sem_id, &set0, 1);
    return 1;
}

void child(int *ptr, int sem_id)
{
    int temp = 0;


    for (int i = 0; i < 10000000; ++i)
        temp += 1;

    if (p(sem_id))
        *ptr += temp;
    
    v(sem_id);
}

int main()
{
    int id = shmget(IPC_PRIVATE, 1, 0666);
    int sem_id = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (sem_id < 0)
    {
        perror("main");
        exit(-1);
    }

    if (id < 0)
    {
        perror("main");
        exit(-1);
    }

    v(sem_id);

    int *ptr = shmat(id, NULL, 0666);

    *ptr = 0;

    for (int i = 0; i < 10; ++i)
    {

        pid_t pid = fork();

        if (pid == 0)
        {
            child(ptr, sem_id);
            exit(-1);
        }
    }

    for (int i = 0; i < 10; ++i)
        wait(NULL);

    printf("%d\n", *ptr);

    shmdt(ptr);
    shmctl(id, IPC_RMID, NULL);
}