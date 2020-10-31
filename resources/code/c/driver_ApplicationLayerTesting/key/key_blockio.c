#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	int fd;
	char status;

	fd = open("/dev/keyDev", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/keyDev\n");
		return -1;
	}

	while(1) {
		read(fd, &status, 1);
		printf("read: %d\n", status);
	}

	close(fd);

	return 0;
}


