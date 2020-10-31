#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd;
	char status;

	fd = open("/dev/ledDev", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/ledDev\n");
		return -1;
	}

	if(!strcmp(argv[1], "on"))
		status = 1;
	else
		status = 0;

	write(fd, &status, 1);
	printf("write: %d\n", status);

	close(fd);

	return 0;
}


