#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

int fd;

void key_sighandler(int num)
{
    char status;

    read(fd, &status, 1);
    printf("read: %d\n", status);
}

int main()
{
	fd = open("/dev/keyDev", O_RDWR | O_NONBLOCK);
	if (fd == -1)
	{
		printf("can not open file /dev/keyDev\n");
		return -1;
	}

    fcntl(fd, F_SETOWN, getpid()); // 将当前进程的进程号告诉给内核
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC);

    signal(SIGIO, key_sighandler); //  设置信号 SIGIO 的处理函数

    while(1);

    close(fd);

    return 0;
}
