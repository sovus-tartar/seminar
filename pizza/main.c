#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sem.h>

int unlock(int sem_id) // unlock
{
    struct sembuf set1 = {0, 1, 0};
    semop(sem_id, &set1, 1);
    return 1;
}

int lock(int sem_id) // lock
{
    struct sembuf set0 = {0, -1, 0};
    semop(sem_id, &set0, 1);
    return 1;
}

int waitzero(int sem_id)
{
    struct sembuf check0 = {0, 0, 0};
    semop(sem_id, &check0, 1);
    return 1;
}

int if_cook_ingr;
int next_pizza;

int cook_ingr(int max)
{

}

int cook_p(int n, int min)
{

}

int cook_i(int n, int min)
{

}

int cook_z(int n, int min)
{

}

int cook_a(int n, int min)
{

}

int client()
{

}

int main(int argc, char ** argv)
{
    if(argc != 4)
    {
        fprintf(stderr, "main: wrong args\n");
        exit(-1);
    }

    int to_cook_num = atoi[1];
    int min_ingr = atoi[2];
    int max_ingr = atoi[3];

    if_cook_ingr = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    next_pizza = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    
    
}