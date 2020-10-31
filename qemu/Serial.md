> 事出有因，在qemu中对ARM串口进行操作时，需要重定向ARM串口到host机


## 1. 重定向ARM串口到host机
* 在qemu-system-arm中加入以下参数
```bash
-nodefaults
-serial stdio     #重定向ARM串口０到host机 stdio
-serial pty       #重定向ARM串口１到host机 /dev/pts/xxx
```


## 2. 操作host机 /dev/pts/xxx
* 打开host机 /dev/pts/xxx
```bash
$ screen /dev/pts/xxx
```

* 暂时退出screen session
```bash
Ctrl-a d
```

* 完全退出某个screen session
进入screen session后，输入以下命令
```bash
$ exit
```

* 查看目前系统存在的screen session
```bash
$ screen -ls
```

* 恢复某个screen session
```bash
$ screen -r [pid]
```


## 3. 操作ARM串口
#### 3.1. linux工具进行测试
* 查看串口参数
```bash
$ stty -a -F /dev/ttyAMA1
```

* 设置串口bandrate
```bash
stty -F /dev/ttyAMA1 115200
```

* 发送数据到串口
```bash
$ echo vernon > /dev/ttyAMA1
```

* 接收串口数据
```bash
$ cat /dev/ttyAMA1
```


### 3.2. C源码进行测试
* 源码如附件１

* 在host shell终端交叉编译serial.c
```bash
$ arm-linux-gnueabi-gcc serial.c -o serial
```

* 进入ARM shell终端执行如下命令
```bash
$ ./serial
```


**附件１:**
serial.c
```c
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
```


