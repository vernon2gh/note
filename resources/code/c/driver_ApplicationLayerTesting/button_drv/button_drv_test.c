#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	int fd;
	char buf;

	/* 1. 判断参数 */
	if (argc != 3)
	{
		printf("Usage: %s </dev/buttonx> -r\n", argv[0]);
		return -1;
	}

	/* 2. 打开文件 */
	fd = open(argv[1], O_RDWR);
	if (fd == -1)
	{
		printf("can not open file %s\n", argv[1]);
		return -1;
	}

	/* 3. 读文件 */
	read(fd, &buf, 1);
	printf("APP read : %d\n", buf);

	close(fd);

	return 0;
}


