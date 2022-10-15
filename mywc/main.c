#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>

enum word_counter_state
{
    NO_WORD,
    IS_WORD
};

enum line_counter_state
{
    NEW_STR,
    OLD_STR
};

int w_state = NO_WORD;
int l_state = NEW_STR;

void wl_count(int *line_count, int *word_count, char *buf, int size)
{
    int count = 0;

    for (int i = 0; i < size; ++i)
    {
        if (isspace(buf[i]))
        {
            if (w_state == IS_WORD)
            {
                w_state = NO_WORD;
            }
        }
        else // no space
        {
            if (w_state == NO_WORD)
            {
                count += 1;
                w_state = IS_WORD;
            }
        }

        if (buf[i] == '\n')
        {
            if (l_state == OLD_STR)
            {
                l_state = NEW_STR;
            }
        }
        else // no new str
        {
            if (l_state == NEW_STR)
            {
                *line_count += 1;
                l_state = OLD_STR;
            }
        }
    }

    *word_count += count;
}

int main(int argc, char **argv)
{
    int fds[2];
    int word_count = 0, byte_count = 0, line_count = 0;
    if (argc == 1)
    {
        return 0;
    }

    pipe(fds);
    pid_t pid = fork();

    if (pid == 0) // child
    {
        close(fds[0]);
        dup2(fds[1], 1);
        close(fds[1]);
        execvp(argv[1], argv + 1);
        exit(0);
    }

    close(fds[1]);
    int temp;
    char buf[16] = {0};

    while (temp = read(fds[0], buf, 16))
    {
        byte_count += temp;

        wl_count(&line_count, &word_count, buf, temp);
    }

    int status;
    wait(&status);

    printf("%d\t%d\t%d\n", byte_count, word_count, line_count);
}