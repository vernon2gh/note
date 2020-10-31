
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

/*
 * ./ledtest -w /dev/led0 on
 * ./ledtest -w /dev/led0 off
 *
 * ./ledtest -r /dev/led0
 *
 */

void usage(char *filename)
{
	printf("Usage: %s <-w/-r> /dev/ledx [on/off]\n", filename);
}

int main(int argc, char **argv)
{
	int fd;
	char status;

	/* 1. 判断参数 */
	if(argc < 3)
	{
		usage(argv[0]);
		return -1;
	}
	else if ((!strcmp(argv[1], "-w")) && (argc != 4))
	{
		usage(argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open(argv[2], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[2]);
		return -1;
	}

	/* 3. 写读文件 */
	if(!strcmp(argv[1], "-w"))
	{
		if(!strcmp(argv[3], "on"))
			status = 1;
		else
			status = 0;

		write(fd, &status, 1);
		printf("write: %d\n", status);
	}
	else if(!strcmp(argv[1], "-r"))
	{
		read(fd, &status, 1);
		printf("read: %d\n", status);
	}
	else
	{
		usage(argv[0]);
	}

	close(fd);

	return 0;
}


