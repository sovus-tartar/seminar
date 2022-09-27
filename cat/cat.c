#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define kb 1024

int cat(int fd_in, int fd_out)
{
	char buf[4 * kb] = "";
	ssize_t char_read = 0;

	while((char_read = read(fd_in, buf, 4 * kb)) != 0)
	{
		if(char_read < 0)
		{
			perror("in cat: ");
			exit(-1);
		}

		ssize_t succ_write = 0;
		
		while ((succ_write = write(fd_out, buf, char_read)) < char_read)
		{
			char_read -= succ_write;
			if (succ_write < 0)
			{
				perror("in cat: ");
				exit(-1);
			}
		}
	}

	return 0;
}


int main(int argc, char  ** argv)
{
	int i = 0;
	switch(argc)
		{
			case 1:
				cat(0, 1);
				break;
			default:
				for(i = 1; i < argc; i++)
					{
						int fd;
						
						fd = open(argv[i], O_RDONLY);
						if (fd < 0)
						{
							perror(argv[i]);
						}
						else
						{
							cat(fd, 1);
						}
						
						close(fd);

					}
				break;
	}

	return 0;
}

