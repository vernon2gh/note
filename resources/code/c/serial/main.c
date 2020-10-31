#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>


int main(void)
{
    int fd, ret;

    fd = open("/dev/ttyAMA1", O_RDWR|O_NOCTTY|O_NONBLOCK);
    if (fd < 0)
    {
        perror("open ttyAMA1");
        return 1;
    }

    fcntl(fd, F_SETFL, 0); //重设为堵塞状态, 去掉O_NONBLOCK

    struct termios opts;
    tcgetattr(fd, &opts); //把原设置获取出来，存放在opts

    //设置波特率
    cfsetispeed(&opts, B19200);
    cfsetospeed(&opts, B19200);

    opts.c_cflag |= CLOCAL|CREAD; //忽略modem控制线, 启动接收器

    // 8N1
    opts.c_cflag &= ~PARENB;
    opts.c_cflag &= ~CSTOPB;
    opts.c_cflag |= CS8;

    opts.c_cflag &= ~CRTSCTS; //关闭硬件流控

    opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //raw input
    opts.c_oflag &= ~OPOST; // raw output

    tcsetattr(fd, TCSANOW, &opts);

    char data[1024];
    char sdata[]="vernon";

    while (1)
    {
        ret = read(fd, data, sizeof(data));
        data[ret] = 0;
        printf("got : %s\n", data);

        write(fd, sdata, sizeof(sdata));
        printf("send data : %s\n", sdata);
    }
    close(fd);
    return 0;
}
