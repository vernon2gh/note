#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;
	unsigned short buf[3];
	unsigned short ir, als, ps;

	if (argc != 2)
	{
		printf("Usage: %s -r\n", argv[0]);
		return -1;
	}

	fd = open("/dev/ap3216cDev", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/ap3216cDev\n");
		return -1;
	}

	read(fd, buf, sizeof(buf));
	ir = buf[0];
	als = buf[1];
	ps = buf[2];
	printf("ir = 0x%x, als = 0x%x, ps = 0x%x\n", ir, als, ps);

	close(fd);

	return 0;
}
