#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <limits.h>

#include <sys/wait.h>

#define _GNU_SOURCE

void str_args(char ** buf, char * line, int size)
{
    int sz = 0;

    for(int i = 0; i <= size; ++i)
    {
        if(isspace(line[i]))
            line[i] = 0;

        if ((line[i] != 0) && ((i == 0)  || (line[i - 1] == 0)))
        {
            ++sz;
            buf[sz - 1] = line + i;
        }

        
    }

    buf[sz] = NULL;
}

void parse_exec(char *line)
{
    int children = 0;
    int fd_to_read = -1;
    if (!strncmp(line, "exit\n", 5))
    {
        exit(0);
    }

    line = strtok(line, "|\n");

    while(line)
    {
        char * curr = line;
        line = strtok(NULL, "|\n"); //may be NULL. if NULL do not create pipe

        char *lex_arr[32768 / 4 - 1];
        str_args(lex_arr, curr, strlen(curr));

        int fds[2];

        if (line)
        {
            if (pipe(fds) == -1) // 0 to read, 1 to write
            {
                perror("shell");
                exit(0);
            } 
        }
            

        pid_t pid = fork();

        if(pid == 0) //child
        {

            if (fd_to_read > 0)
            {
                dup2(fd_to_read, 0);
                close(fd_to_read);
            }
            
            if (line)
            {
                close(fds[0]);
                dup2(fds[1], 1);
                close(fds[1]);
            }

            if (execvp(lex_arr[0], lex_arr) == -1)
            {
                perror("shell");
                fprintf(stderr, "%s\n", curr);
                exit(-1);
            }
        } 
        else //shell proccess
        {
            if (fd_to_read > 0)
            {
                close(fd_to_read);
            }
            children += 1;
            if(line)
            {
                close(fds[1]);
                fd_to_read = fds[0];
            }
        }

    }

    for(int i = 0; i < children; ++i)
        wait(NULL);

}

int main()
{
    while (1)
    {
        char *ptr = NULL;
        size_t n;

        putchar('>');
        
        getline(&ptr, &n, stdin);
        parse_exec(ptr);
        free(ptr);
    }
}