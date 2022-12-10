#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <assert.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>

#define NEXT_ 1000000

int bridge_sem;

int *line_num_arr;
int line_num_sem;

int queue_id[2];

struct msg_t
{
    long msg_type;
    int msg_data;
};

int
v(int sem_id, int sem_num) // unlock
{
    struct sembuf set1 = {sem_num, 1, 0};
    if (semop(sem_id, &set1, 1) == -1)
    {
        perror("v");
        exit(-1);
    }

    return 1;
}

int p(int sem_id, int sem_num) // lock
{
    struct sembuf set0 = {sem_num, -1, 0};
    if (semop(sem_id, &set0, 1) == -1)
    {
        perror("p");
        exit(-1);
    }
    return 1;
}

int wo(int sem_id, int sem_num) //wait_one
{
    struct sembuf check1[2] = {{sem_num, -1, 0}, {sem_num, 1, 0}};
    if (semop(sem_id, check1, 2) == -1)
    {
        perror("wz");
        exit(-1);
    }
    return 1;
}

int pastux(int num, int side, int times);

int init(int n0, int n1, int times)
{
    bridge_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if (bridge_sem == -1)
    {
        perror("bridge_init_failed");
        exit(-1);
    }

    v(bridge_sem, 0);

    int line_num_key = shmget(IPC_PRIVATE, 2 * sizeof(int), 0666);
    if (line_num_key == -1)
    {
        perror("line init failed");
        exit(-1);
    }

    line_num_arr = shmat(line_num_key, NULL, 0666);
    line_num_arr[0] = 0;
    line_num_arr[1] = 0;

    line_num_sem = semget(IPC_PRIVATE, 2, 0666 | IPC_CREAT);
    if (line_num_sem == -1)
    {
        perror("line sem init failed");
        exit(-1);
    }

    v(line_num_sem, 0);
    v(line_num_sem, 1);

    key_t key[2] = {100, 101};
    queue_id[0] = msgget(key[0], IPC_CREAT | 0666);
    queue_id[1] = msgget(key[1], 0666 | IPC_CREAT);
    if ((queue_id[0] == -1) || (queue_id[1] == -1))
    {
        perror("line msg init failed");
        exit(-1);
    }

    for (int i = 1; i <= n0; ++i)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            pastux(i, 0, times);
            exit(0);
        }
    }

    for (int i = n0; i <= n1 + n0; ++i)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            pastux(i, 1, times);
            exit(0);
        }
    }


}

int change_side(int side)
{
    return ((side + 1) % 2);
}

int pastux(int num, int side, int times)
{
    printf("Pastux #%d was born on the side %d\n", num, side);

    for (int i = 0; i < times; ++i)
    {

        printf("Pastux #%d: looking for others on the side %d\n", num, side);
        if (p(line_num_sem, side))
        {
            printf("Pastux #%d: lna %d\n", num, line_num_arr[side]);
            if (line_num_arr[side] != 0)
            {
                line_num_arr[side] += 1;
                printf("Pastux #%d: somebody on the side %d, waiting...\n", num, side);
                struct msg_t message = {NEXT_, num};
                v(line_num_sem, side);

                msgsnd(queue_id[side], &message, sizeof(int), 0);
                msgrcv(queue_id[side], NULL, 0, num, 0); // waiting for turn
            }
            else
            {
                line_num_arr[side] += 1;
                printf("Pastux #%d: nobody on the side %d\n", num, side);
                v(line_num_sem, side);
            }
        }

        // definitely our turn

        if (side == 0)
        {
            printf("Pastux #%d: run from side %d to other side to put hat\n", num, side);
            p(bridge_sem, 0);
            printf("Pastux #%d: hat put, running back to side %d\n", num, side);
        }
        else //side == 1 
        {
            printf("Pastux #%d: looking for hat...\n", num);
            wo(bridge_sem, 0);
            printf("Pastux #%d: no hat, going...\n", num);
        }

        printf("Pastux #%d: take all his cows from %d to the other side\n", num, side);
        if (p(line_num_sem, side))
        {
            line_num_arr[side] -= 1;
            v(line_num_sem, 0);
        }

        int num_of_next = -1;
        msgrcv(queue_id[side], &num_of_next, sizeof(int), 0, IPC_NOWAIT);
        if (num_of_next != -1)
        {
            long msg_to_next = num_of_next;
            msgsnd(queue_id[side], &msg_to_next, 0, 0);
        }

        side = change_side(side);

        printf("Pastux #%d: changed side to %d\n", num, side);

        if (side == 1)
        {
            printf("Pastux #%d: removing hat\n", num);
            v(bridge_sem, 0);
            printf("Pastux #%d: hat has been removed\n", num);
        }
    }
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    int n0 = atoi(argv[1]);
    int n1 = atoi(argv[2]);
    int times = atoi(argv[3]);

    init(n0, n1, times);

    for(int i = 0; i < (n0 + n1); ++i)
    {
        wait(NULL);
    }

    return 0;
}