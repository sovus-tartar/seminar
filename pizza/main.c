#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <string.h>

enum ingridients
{
    P_LETTER,
    I_LETTER,
    Z_LETTER,
    A_LETTER
};

int v(int sem_id) // unlock
{
    struct sembuf set1 = {0, 1, 0};
    if (semop(sem_id, &set1, 1) == -1)
    {
        perror("v");
        exit(-1);
    }

    return 1;
}

int p(int sem_id) // lock
{
    struct sembuf set0 = {0, -1, 0};
    if (semop(sem_id, &set0, 1) == -1)
    {
        perror("p");
        exit(-1);
    }
    return 1;
}

int wz(int sem_id) //wait_zero
{
    struct sembuf check0 = {0, 0, 0};
    if (semop(sem_id, &check0, 1) == -1)
    {
        perror("wz");
        exit(-1);
    }
    return 1;
}

int wo(int sem_id) //wait_one
{
    struct sembuf check1[2] = {{0, -1, 0}, {0, 1, 0}};
    if (semop(sem_id, check1, 2) == -1)
    {
        perror("wz");
        exit(-1);
    }
    return 1;
}

int cz(int sem_id) //check_zero
{
    struct sembuf check0 = {0, 0, IPC_NOWAIT};
    if (semop(sem_id, &check0, 1) == -1)
        return 0;
    return 1;
}

int p_sem;
int i_sem;
int zz_sem;
int a_sem;

int low_ingr_sem;

char *conv;
int conv_sem;

int *ingr_arr;
int ingr_sem;

int success_pizzas = 0;

int stop_work_sem;

char get_ingr_letter(int i)
{
        switch(i)
        {
            case P_LETTER:
                return 'P';
            case I_LETTER:
                return 'I';
            case Z_LETTER:
                return 'Z';
            case A_LETTER:
                return 'A';
        }
}

int take_ingridient(int name, int min)
{
    p(ingr_sem);

    if(ingr_arr[name] > 0)
        ingr_arr[name] -= 1;
    else
    {
        printf("Calling ingr_filler. Waiting for him. (left: %d, letter: %c)\n", ingr_arr[name], get_ingr_letter(name));
        v(low_ingr_sem);
        wz(low_ingr_sem);
        ingr_arr[name] -= 1;
    }

    if (ingr_arr[name] <= min)
    {
        printf("Calling ingr_filler. Not waiting for him. (left: %d letter: %c)\n", ingr_arr[name], get_ingr_letter(name));
        v(low_ingr_sem);
    }

    v(ingr_sem);
    
    return 1;
}

void print_conv_state()
{
    printf("1: %s\n2: %s\n3: %s\n4: %s\n", conv, conv + 6, conv + 12, conv + 18);
}

int ingr_filler(int min, int max)
{
    printf("ingr_filler: started\n");

    while(1)
    {
        wo(low_ingr_sem);

        if(cz(stop_work_sem))
        {
            printf("ingr_filler: died\n");
            exit(0);
        }

        printf("Ingr_filler has arrived\n");
        int i;

        for(i = 0; i < 4; ++i)
            if(ingr_arr[i] < min)
            {
                ingr_arr[i] = max;
                break;
            }

        char c = get_ingr_letter(i);

        printf("Ingr_filler has filled %c ingridient\n", c);

        p(low_ingr_sem);
    }
}

int cook_p(int min)
{
    printf("cooker_p started\n");
    int first_time_check = 1;

    while (1)
    {
        wz(p_sem);

        if(cz(stop_work_sem))
        {
            printf("cook_p: died\n");
            exit(0);
        }

        take_ingridient(P_LETTER, min);
        printf("coocker_p: took P ingridient\n");
        conv[6 * 0 + 0] = 'P';
        printf("cooker_p: put P to the conv\n");

        v(p_sem);
    }
}

int cook_i(int min)
{
    printf("cooker_i started\n");
    int first_time_check = 1;

    while (1)
    {
        wz(i_sem);
        
        if(cz(stop_work_sem))
        {
            printf("cook_i: died\n");
            exit(0);
        }

        if(conv[6 * 1] != 0)
        {
            take_ingridient(I_LETTER, min);
            printf("Coocker_i: took i ingridient\n");
            conv[6 * 1 + 1] = 'I';
            printf("Coocker_i: put i to the conv\n");
        }
        else
        {
            printf("cook_i: no pizza, ignoring\n");
        }

       v(i_sem);
    }
}

int cook_z(int min)
{
    printf("cooker_z started\n");
    int first_time_check = 1;

    while (1)
    {
        wz(zz_sem);

        if(cz(stop_work_sem))
        {
            printf("cook_z: died\n");
            exit(0);
        }

        if (conv[6 * 2 + 1] != 0)
        {
            take_ingridient(Z_LETTER, min);
        printf("Coocker_z: took z ingridient\n");

        conv[6 * 2 + 2] = 'Z';
        printf("Coocker_z: put first z to conv\n");

        // print_conv_state();

        take_ingridient(Z_LETTER, min);
        printf("Coocker_z: took z ingridient\n");

        // print_conv_state();

        conv[6 * 2 + 3] = 'Z';
        printf("Coocker_z: put second z to conv\n");
        }
        else
        {
            printf("cook_z: no pizza, ignoring\n");
        }

        

        v(zz_sem);
    }
}

int cook_a(int min)
{
    printf("cooker_a started\n");
    int first_time_check = 1;
    while (1)
    {
        wz(a_sem);

        if(cz(stop_work_sem))
        {
            printf("Cook_a: died\n");
            exit(0);
        }

        if(conv[6 * 3 + 2] != 0)
        {
            take_ingridient(A_LETTER, min);
            printf("Coocker_a: took a ingridient\n");

            conv[6 * 3 + 4] = 'A';
            printf("Coocker_a: put a to conv\n");
        }
        else
        {
            printf("cook_a: no pizza, ignoring\n");
        }

        

        v(a_sem);
    }
}

int client(char *food)
{

    printf("Client will recieve: %s\n", food);
    if (!strncmp(food, "PIZZA", 5))
    {
        ++success_pizzas;
    }
}

void shift_conv()
{
    for (int i = 3; i > 0; --i)
    {
        if ((i == 3) && (conv[6 * i] != 0))
        {
            client(conv + 6 * i);
        }

        strncpy(conv + 6 * i, conv + 6 * (i - 1), 5);
    }

    for (int j = 0; j < 6; ++j)
    {
        conv[j] = 0;
    }

    printf("Conv shifted!\n");
    print_conv_state();
}

int suspend_processes()
{
    p(stop_work_sem);

    p(p_sem);
    wait(NULL);
    p(i_sem);
    wait(NULL);
    p(zz_sem);
    wait(NULL);
    p(a_sem);
    wait(NULL);

    v(low_ingr_sem);
    wait(NULL);
}

int daemon_shifter(int n, int min, int max) //daemon shifter is like a manager)
{
    printf("Daemon shifter: started\n");

    pid_t pid = fork();

    if(pid == 0)
    {
        ingr_filler(min, max);
        exit(0);
    }


    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 6; ++j)
        {
            conv[i * 6 + j] = 0;
        }

    pid = fork();

    if (pid == 0)
    {
        cook_p(min);
        exit(0);
    }

    pid = fork();

    if (pid == 0)
    {
        cook_i(min);
        exit(0);
    }


    pid = fork();

    if (pid == 0)
    {
        cook_z(min);
        exit(0);
    }


    pid = fork();

    if (pid == 0)
    {
        cook_a(min);
        exit(0);
    }



    for (int i = 0; i <= n + 3; ++i)
    {
        if (((wo(p_sem) && wo(i_sem)) && (wo(zz_sem) && wo(a_sem))))
        {
            if (i < n + 3)
            {
                shift_conv();
                p(p_sem);
                p(i_sem);
                p(zz_sem);
                p(a_sem);
            }
            else
            {
                suspend_processes();
            }
            
        }
    }
}



int main(int argc, char **argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "main: wrong args\n");
        exit(-1);
    }

    int to_cook_num = atoi(argv[1]);
    int min_ingr = atoi(argv[2]);
    int max_ingr = atoi(argv[3]);

    int conv_key = shmget(IPC_PRIVATE, 4 * 6 * sizeof(char), 0666);
    if (conv_key == -1)
    {
        perror("conv creation failed");
        exit(-1);
    }

    conv = shmat(conv_key, NULL, 0666);

    if (conv == NULL)
    {
        perror("shared conv address failed");
        exit(-1);
    }

    conv_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (conv_sem == -1)
    {
        perror("conv_sem creation failed");
        exit(-1);
    }

    int ingr_arr_key = shmget(IPC_PRIVATE, 4 * sizeof(int), 0666);
    if (ingr_arr_key == -1)
    {
        perror("failed to create ingr_arr");
        exit(-1);
    }

    ingr_arr = shmat(ingr_arr_key, NULL, 0666);

    if (ingr_arr == NULL)
    {
        perror("ingr shared arr creation failed");
    }

    conv_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (conv_sem == -1)
    {
        perror("conv_sem creation failed");
        exit(-1);
    }

    p_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (p_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    //v(p_sem);

    i_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (i_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    //v(i_sem);

    zz_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (zz_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    //v(zz_sem);

    a_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (a_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    //v(a_sem);

    low_ingr_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (low_ingr_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    v(low_ingr_sem);

    ingr_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (ingr_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    v(ingr_sem);

    stop_work_sem = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);

    if (stop_work_sem == -1)
    {
        perror("semaphore init failed");
        exit(-1);
    }

    v(stop_work_sem);

    daemon_shifter(to_cook_num, min_ingr, max_ingr);

    printf("Success pizzas: %d\n", success_pizzas);

    return 0;

    // if_cook_ingr = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
    // next_pizza = semget(IPC_PRIVATE, 1, 0666 | IPC_CREAT);
}