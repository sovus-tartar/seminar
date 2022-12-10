#define _GNU_SOURCE

#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/time.h>

struct buffer
{
    char buff[4096];
    int sz;
};

int count_checksum(char * string, int n)
{
    int sum = 0;

    for(int i = 0; i < n ; ++i)
        sum += string[i];

    return sum;
}

int master(int n, char *name)
{
    int fd_arr[n + 1][2]; // 0 - to read, 1 - to write, n - num of child
    int control_sum_arr[n][2];
    struct buffer buffer_arr[n + 1];
    // initialization

    for (int i = 0; i < n + 1; ++i)
    {
        buffer_arr[i].sz = 0;
    }
    for(int i = 0; i < n; ++i)
    {
        control_sum_arr[i][0] = 0;
        control_sum_arr[i][1] = 0;
    }
    fd_arr[0][0] = 0;
    fd_arr[n][1] = 1;

    for (int i = 1; i <= n; ++i)
    {
        int fds_in[2], fds_out[2]; // out from master, in to master

        pipe2(fds_in, O_NONBLOCK);
        pipe2(fds_out, O_NONBLOCK);

        fd_arr[i][0] = fds_in[0];
        fd_arr[i - 1][1] = fds_out[1];

        pid_t pid = fork();

        if (pid == 0) // child cat
        {

            int in_status = fcntl(fds_in[1], F_GETFL) & ~O_NONBLOCK;
            int out_status = fcntl(fds_out[0], F_GETFL) & ~O_NONBLOCK;

            fcntl(fds_in[1], F_SETFL, in_status);
            fcntl(fds_out[0], F_SETFL, out_status);

            close(fd_arr[0][1]);
            close(fd_arr[n][0]);
            for (int j = 1; j < i; ++j)
            {
                close(fd_arr[j][0]);
                close(fd_arr[j][1]);
            }

            dup2(fds_out[0], 0);
            close(fds_out[0]);
            dup2(fds_in[1], 1);
            close(fds_in[1]);

            int exec_err = execlp(name, name, NULL);

            if (exec_err)
            {
                perror(name);
                exit(-1);
            }

            exit(0);
        }

        // master
        close(fds_in[1]);
        close(fds_out[0]);
    }

    int status = 1;
    int input_finished = 0;

    struct timeval tv1, tv2;
    struct timezone tz1, tz2;

    while (status)
    {
        int activity = 0;

        for (int i = 0; i < n + 1; ++i)
        {
            int bytes_read;

            if (buffer_arr[i].sz <= 0)
            {
                if (((!input_finished) && (i == 0)) || (i > 0))
                {
                    bytes_read = read(fd_arr[i][0], buffer_arr[i].buff, 4096);
                    buffer_arr[i].sz = bytes_read;
                    if (i != 0)
                    {
                        control_sum_arr[i - 1][1] += count_checksum(buffer_arr[i].buff, bytes_read);
                    }
                }

                if ((bytes_read == 0) && (i == 0))
                {
                    input_finished = 1;
                    printf("Waiting 2 seconds for pipes to become empty...\n");
                    gettimeofday(&tv1, &tz1);
                }
            }

            if (buffer_arr[i].sz > 0)
            {
                if (write(fd_arr[i][1], buffer_arr[i].buff, buffer_arr[i].sz) != -1)
                {
                     if(i != n)
                    {
                        control_sum_arr[i][0] += count_checksum(buffer_arr[i].buff, buffer_arr[i].sz);
                    }
                    buffer_arr[i].sz = 0;
                }
            }
        }

        if (input_finished)
        {
            gettimeofday(&tv2, &tz2);
            if (tv2.tv_sec - tv1.tv_sec >= 2)
                status = 0;
        }
    }

    close(fd_arr[0][1]);
    close(fd_arr[n][0]);
    for(int i = 1; i < n; ++i)
    {
        close(fd_arr[i][1]);
        close(fd_arr[i][0]);
    }
        


    for (int i = 0; i < n; ++i)
        wait(NULL);

    for(int i = 0; i < n; ++i)
    {
        printf("Checksum for program #%d\nin: %d    out: %d\n", i, control_sum_arr[i][0], control_sum_arr[i][1]);
    }
}

int main(int argc, char **argv)
{
    assert(argc > 1);

    int num_of_cats = atoi(argv[2]);
    char *name = argv[1];

    master(num_of_cats, name);
}