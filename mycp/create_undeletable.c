#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


int main()
{
	int fd = open("test_unremovable.test", O_CREAT | O_RDONLY, S_IRUSR);

	write(fd, "Hello, undeletable file!", 25);

	close(fd);

	return 0;
}
