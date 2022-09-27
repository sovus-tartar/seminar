#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>


int main()
{
	open("test_unremovable.test", O_CREAT | O_RDONLY, S_IRUSR);
}
