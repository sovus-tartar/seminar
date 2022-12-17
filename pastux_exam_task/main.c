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

#define TO_QUEUE 1000000
#define LEAVE_QUEUE 1000001

int bridge_sem;

int entry_sem;
int end_sem;
int *line_num_arr;
int line_num_sem;

int queue_id[2];

struct msg_t
{
    long msg_type;
    int msg_data;
};

int v(int sem_id, int sem_num) // unlock
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

int wo(int sem_id, int sem_num) // wait_one
{
    struct sembuf check1[2] = {{sem_num, -1, 0}, {sem_num, 1, 0}};
    if (semop(sem_id, check1, 2) == -1)
    {
        perror("wz");
        exit(-1);
    }
    return 1;
}

int queue_daemon(int side)
{
    while (1)
    {
        struct msg_t rcv1;
        msgrcv(queue_id[side], &rcv1, sizeof(int), TO_QUEUE, 0);

        if(rcv1.msg_data == LEAVE_QUEUE)
        {
            printf("queue_daemon %d died\n", side);
            exit(0);
        }

        struct msg_t snd = {rcv1.msg_data, 0};

        msgsnd(queue_id[side], &snd, sizeof(int), 0);

        struct msg_t rcv2;

        msgrcv(queue_id[side], &rcv2, sizeof(int), LEAVE_QUEUE, 0);
    }
}

void kill_queue(int num)
{
    struct msg_t snd = {TO_QUEUE, LEAVE_QUEUE};
    
    msgsnd(queue_id[num], &snd, sizeof(int), 0);
    printf("Msg send to %d\n", num);
}

int stand_to_queue(int num, int side)
{
    struct msg_t snd = {TO_QUEUE, num};
    msgsnd(queue_id[side], &snd, sizeof(int), 0);

    struct msg_t rcv;
    msgrcv(queue_id[side], &rcv, sizeof(int), num, 0);
    return 0;
}

int leave_queue(int num, int side)
{
    struct msg_t snd = {LEAVE_QUEUE, num};
    msgsnd(queue_id[side], &snd, sizeof(int), 0);
    return 0;
}

void pastux(int num, int side, int times);

void init(int n0, int n1, int times)
{
    end_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    if(end_sem == -1)
    {
        perror("end_sem init failed");
        exit(-1);
    }

    entry_sem = semget(IPC_PRIVATE, 2, 0666 | IPC_CREAT);
    if (entry_sem == -1)
    {
        perror("entry_sem init failed");
        exit(-1);
    }

    v(entry_sem, 0);
    v(entry_sem, 1);

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

    for(int i = 0; i < 2; ++i)
    {
        pid_t pid = fork();

        if(pid == 0)
        {
            queue_daemon(i);
        }
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

    for(int i = 0; i <= (n1 + n0); ++i)
        p(end_sem, 0);
    
    for(int i = 0; i < 2; ++i)
        kill_queue(i);

    for(int i = 0; i < (n1 + n0 + 2); ++i)
    {
        wait(NULL);
    }
}

int change_side(int side)
{
    return ((side + 1) % 2);
}

void pastux(int num, int side, int times)
{
    printf("Pastux #%d was born on the side %d\n", num, side);

    for (int i = 0; i < times; ++i)
    {
        printf("Pastux #%d: looking for others on the side %d\n", num, side);
        stand_to_queue(num, side);
        printf("Pastux #%d: time to go\n", num);
        p(entry_sem, side);
        // definitely our turn

        if (side == 0)
        {
            printf("Pastux #%d: run from side %d to other side to put hat\n", num, side);
            wo(entry_sem, change_side(side));
            p(bridge_sem, 0);
            printf("Pastux #%d: hat put, running back to side %d\n", num, side);
        }
        else // side == 1
        {
            printf("Pastux #%d: looking for hat...\n", num);
            wo(bridge_sem, 0);
            printf("Pastux #%d: no hat, going...\n", num);
        }

        printf("Pastux #%d: take all his cows from %d to the other side\n", num, side);
        leave_queue(num, side);
        v(entry_sem, side);

        side = change_side(side);

        printf("Pastux #%d: changed side to %d\n", num, side);

        if (side == 1)
        {
            printf("Pastux #%d: removing hat\n", num);
            v(bridge_sem, 0);
            printf("Pastux #%d: hat has been removed\n", num);
        }
    }

    printf("Pastux #%d: has died\n", num);
    v(end_sem, 0);
    exit(0);
}

int main(int argc, char **argv)
{
    assert(argc == 4);
    int n0 = atoi(argv[1]);
    int n1 = atoi(argv[2]);
    int times = atoi(argv[3]);

    init(n0, n1, times);

    for (int i = 0; i < (n0 + n1); ++i)
    {
        wait(NULL);
    }

    return 0;
}