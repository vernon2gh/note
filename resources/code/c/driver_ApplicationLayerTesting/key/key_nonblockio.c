#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/select.h>
#include <poll.h>

#define POLL_METHOD

int main(int argc, char **argv)
{
#ifdef SELECT_METHOD
	fd_set readfds;
	int nfds;
	struct timeval timeout;
#endif

#ifdef POLL_METHOD
	struct pollfd fds;
	nfds_t nfds;
	int timeout;
#endif

	int fd, ret;
	char status;

	fd = open("/dev/keyDev", O_RDWR | O_NONBLOCK);
	if (fd == -1)
	{
		printf("can not open file /dev/keyDev\n");
		return -1;
	}

#ifdef SELECT_METHOD
	while(1) {
		FD_ZERO(&readfds);
		FD_SET(fd, &readfds);
		nfds = fd + 1;

		timeout.tv_sec = 3;
		timeout.tv_usec = 0;

		ret = select(nfds, &readfds, NULL, NULL, &timeout);
		if(ret > 0)
		{
			if(FD_ISSET(fd, &readfds)) {
				read(fd, &status, 1);
				printf("read: %d\n", status);
			}
			else {
				printf("no this fd!\n");
			}
		}
		else if(ret == 0)
		{
			printf("timeout!\n");
		}
		else
		{
			printf("error!\n");
		}
	}
#endif

#ifdef POLL_METHOD
	fds.fd = fd;
	fds.events = POLLIN;
	nfds  = 1;

	timeout = 500;

	while(1) {
		ret = poll(&fds, nfds, timeout);
		if(ret > 0)
		{
			read(fd, &status, 1);
			printf("read: %d\n", status);
		}
		else if(ret == 0)
		{
			printf("timeout!\n");
		}
		else
		{
			printf("error!\n");
		}
	}
#endif

	close(fd);

	return 0;
}


