#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int fd;
	unsigned short buf[7];

	if (argc != 2)
	{
		printf("Usage: %s -r\n", argv[0]);
		return -1;
	}

	fd = open("/dev/icm20608Dev", O_RDWR);
	if (fd == -1)
	{
		printf("can not open file /dev/icm20608Dev\n");
		return -1;
	}

	read(fd, buf, sizeof(buf));
	printf("accel_x_adc : %d\n", buf[0]);
	printf("accel_y_adc : %d\n", buf[1]);
	printf("accel_z_adc : %d\n", buf[2]);
	printf("temp_adc    : %d\n", buf[3]);
	printf("gyro_x_adc  : %d\n", buf[4]);
	printf("gyro_y_adc  : %d\n", buf[5]);
	printf("gyro_z_adc  : %d\n", buf[6]);

	close(fd);

	return 0;
}
