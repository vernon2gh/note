## 简述

linux kernel 阻塞IO/非阻塞IO/异步通知的编写，都分为三步：

1. 创建 struct file_operations 变量
2. 创建 阻塞IO/非阻塞IO/异步通知
3. 唤醒
4. 编写应用程序

### 阻塞IO

#### 1. 创建 struct file_operations 变量

```c
const struct file_operations xxx_fops = {
	.read    = xxx_read,
};
```

#### 2. 创建阻塞IO

```c
DECLARE_WAIT_QUEUE_HEAD(xxx_waitQueueHead); // 定义及初始化等待队列头

ssize_t xxx_read(struct file *file, char __user *buf, size_t size, loff_t *offset)
{
	unsigned char ret;

	if(file->f_flags & O_NONBLOCK)
	{
		// 非阻塞IO模式
	}
	else
	{
		// 阻塞IO模式
		DECLARE_WAITQUEUE(wq, current);            // 创建等待队列
		add_wait_queue(&xxx_waitQueueHead, &wq);   // 将 等待队列 加入 等待队列头
		set_current_state(TASK_INTERRUPTIBLE);
		schedule();                                // 睡眠，等待唤醒

		remove_wait_queue(&xxx_waitQueueHead, &wq);// 唤醒后，从 等待队列头 删除 等待队列
	}

	......

	// 将内核空间kernel_buffer复制到用户空间buf
	ret = copy_to_user(buf, &kernel_buffer, size); 

	return 0;
}
```

#### 3. 唤醒

唤醒步骤一般在中断处理函数或下半部函数中调用

```c
wake_up(&xxx_waitQueueHead);
```

#### 4. 编写应用程序

```c
int main(int argc, char **argv)
{
	int fd;
	char status;

	fd = open("/dev/xxxDev", O_RDWR); // 默认以阻塞IO模式打开

	while(1) {
		read(fd, &status, 1); // 调用kernel xxx_read()
		printf("read: %d\n", status);
	}

	close(fd);
	return 0;
}
```

### 非阻塞IO

#### 1. 创建 struct file_operations 变量

```c
const struct file_operations xxx_fops = {
	.poll    = xxx_poll,
};
```

#### 2. 创建非阻塞IO

```c
DECLARE_WAIT_QUEUE_HEAD(xxx_waitQueueHead); // 定义及初始化等待队列头

unsigned int xxx_poll(struct file *file, struct poll_table_struct *wait)
{
	poll_wait(file, &xxx_waitQueueHead, wait); // 睡眠，等待唤醒

	return xxx_val? POLLIN : 0;
}
```

#### 3. 唤醒

唤醒步骤一般在中断处理函数或下半部函数中调用

```c 
wake_up(&xxx_waitQueueHead);
```

#### 4. 编写应用程序

* SELECT_METHOD

  ```c
  #include <sys/select.h>
  
  int main(int argc, char **argv)
  {
  	fd_set readfds;
  	int nfds;
  	struct timeval timeout;
  	int fd, ret;
  	char status;
  
  	fd = open("/dev/xxxDev", O_RDWR | O_NONBLOCK); // 以非阻塞IO模式打开
  
  	while(1) {
  		FD_ZERO(&readfds);   // 置零读文件描述符
  		FD_SET(fd, &readfds);// 将fd加入读文件描述符
  		nfds = fd + 1;       // 设置读文件描述符的个数
  
  		timeout.tv_sec = 3; // 3秒超时
  		timeout.tv_usec = 0;
  
  		/*
  		 * 调用kernel xxx_poll()
  		 * 如果3秒内，有数据可读，调用read()
  		 * 如果3秒内，没有数据可读，返回超时状态
  		 */
  		ret = select(nfds, &readfds, NULL, NULL, &timeout); 
  		if(ret > 0)
  		{
  			if(FD_ISSET(fd, &readfds)) {
  				read(fd, &status, 1); // 调用kernel xxx_read()
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
  
  	close(fd);
  	return 0;
  }
  ```

* POLL_METHOD

  ```c
  #include <poll.h>
  
  int main(int argc, char **argv)
  {
  	struct pollfd fds;
  	nfds_t nfds;
  	int timeout;
  	int fd, ret;
  	char status;
  
  	fd = open("/dev/xxxDev", O_RDWR | O_NONBLOCK); // 以非阻塞IO模式打开
  
  	fds.fd = fd;
  	fds.events = POLLIN; // 监听POLLIN事件
  	nfds  = 1;
  
  	timeout = 500;       // 500ms超时
  
  	while(1) {
  		/*
  		 * 调用kernel xxx_poll()
  		 * 如果500ms内，有数据可读，调用read()
  		 * 如果500ms内，没有数据可读，返回超时状态
  		 */
  		ret = poll(&fds, nfds, timeout);
  		if(ret > 0)
  		{
  			read(fd, &status, 1); // 调用kernel xxx_read()
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
  
  	close(fd);
  	return 0;
  }
  ```

### 异步通知

#### 1. 创建 struct file_operations 变量

```c
const struct file_operations xxx_fops = {
	.fasync  = xxx_fasync,
};
```

#### 2. 创建异步通知

```c
struct fasync_struct *fasync_queue; // 定义异步通知

int xxx_fasync(int fd, struct file *file, int on)
{
	return fasync_helper(fd, file, on, &fasync_queue); // 初始化异步通知
}
```

#### 3. 唤醒

唤醒步骤一般在中断处理函数或下半部函数中调用

```c
kill_fasync(&fasync_queue, SIGIO, POLL_IN); // 向应用程序发送SIGIO信号
```

#### 4. 编写应用程序

```c
#include <signal.h>

int fd;

void xxx_sighandler(int num)
{
	char status;

	read(fd, &status, 1); // 调用kernel xxx_read()
	printf("read: %d\n", status);
}

int main()
{
	fd = open("/dev/xxxDev", O_RDWR | O_NONBLOCK); // 以非阻塞IO模式打开

	fcntl(fd, F_SETOWN, getpid()); // 将当前进程的进程号告诉给内核
    
	// 设置异步通知模式，调用kernel xxx_fasync()
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | FASYNC); 

	/*
	 * 设置SIGIO信号的处理函数为xxx_sighandler()
	 *
	 * 当kernel调用kill_fasync()向应用程序发送SIGIO信号时，
	 * 应用程序调用xxx_sighandler()
	 */
	signal(SIGIO, xxx_sighandler);

	while(1);
	close(fd);
	return 0;
}
```

