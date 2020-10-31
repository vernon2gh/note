### fork()、vfork()、pthread_create()

* 测试环境：

ubuntu 10.04 ( linux 2.6.32 )

* 测试：

```bash
$ cat test.c
#include <sched.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int data = 10;

int child_process()
{
	printf("Child process %d, data %d\n",getpid(),data);
	data = 20;
	printf("Child process %d, data %d\n",getpid(),data);
	_exit(0);
}

int main(int argc, char* argv[])
{
	int pid;
	// pid = fork();
	// pid = vfork();

	if(pid==0) {
		child_process();
	}
	else{
		sleep(1);
		printf("Parent process %d, data %d\n",getpid(), data);
		exit(0);
	}
}

$ gcc test.c -o test

# 当pid = fork();时
$ ./test
Child process 2646, data 10
Child process 2646, data 20
Parent process 2645, data 10
$ strace ./test
...
clone(child_stack=x, flags=CLONE_CHILD_CLEARTID|CLONE_CHILD_SETTID|SIGCHLD, child_tidptr=xxx)
...

# 当pid = vfork();时
$ ./test
Child process 2702, data 10
Child process 2702, data 20
Parent process 2701, data 20
$ strace ./test
xxxxx

$ cat thread.c 
#include <stdio.h>
#include <pthread.h>
#include <stdio.h>
#include <linux/unistd.h>
#include <sys/syscall.h>

int data = 10;

static pid_t gettid( void )
{
	return syscall(__NR_gettid);
}

static void *thread_fun(void *param)
{
	data = 20;

	printf("thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(),pthread_self());
	printf("data %d\n", data);
	return NULL;
}

int main(void)
{
	pthread_t tid;
	int ret;

	printf("thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(),pthread_self());
	printf("data %d\n", data);

	ret = pthread_create(&tid, NULL, thread_fun, NULL);
	if (ret == -1) {
		perror("cannot create new thread");
		return -1;
	}

	if (pthread_join(tid, NULL) != 0) {
		perror("call pthread_join function fail");
		return -1;
	}

	printf("thread pid:%d, tid:%d pthread_self:%lu\n", getpid(), gettid(),pthread_self());
	printf("data %d\n", data);

	return 0;
}
$ gcc thread.c -pthread -o thread
$ ./thread
thread pid:1878, tid:1878 pthread_self:140648953210624
data 10
thread pid:1878, tid:1879 pthread_self:140648945157904
data 20
thread pid:1878, tid:1878 pthread_self:140648953210624
data 20
$ strace ./thread
...
clone(child_stack=xxx,flags=CLONE_VM|CLONE_FS|CLONE_FILES|CLONE_SIGHAND|CLONE_THREAD|CLONE_SYSVSEM|CLONE_SETTLS|CLONE_PARENT_SETTID|CLONE_CHILD_CLEARTID, parent_tidptr=xxx,tls=xxx, child_tidptr=xxx) = xxx
...
```

* 总结：

```bash
用户空间 -------------------------->  内核空间
fork()           -> clone(flag) -> do_fork(flag)
vfork()          -> clone(flag) -> do_fork(flag)
pthread_create() -> clone(flag) -> do_fork(flag)
```

如上图，无论创建进程还是线程，都是调用系统调用`clone ()`，再调用内核函数`do_fork()`创建`struct task_strust`，唯一不同是它们传递`flag`标志不同，以区分哪些资源是否共享等等。

linux使用 `fork()` 创建进程时，父进程与子进程共享同一个资源，即`struct task_strust` 指向同一个资源，并此时所有资源都以只读方式共享。只有 父进程或子进程 其中一个对资源进行**写操作**时，linux才会为 父进程或子进程 创建复制一份一模一样的资源，即 此时 父子进程 才各自拥有一份完全不同的`struct task_strust`，对资源都拥有读写权限。这就是进程写时拷贝(Copy-on-Write)技术。

linux使用 `vfork()` 创建进程时，跟`fork()` 创建进程基本是一样的，唯一不同是`task_strust－>mm`成员是共享，如 进程中全局变量是共享。

linux使用`pthread_create()`创建线程时，进程内所有线程共享同一个资源，即 `struct task_strust` 指向同一个资源，所有资源都以读写方式共享。

### 为什么 线程称为轻量级进程？

从上述可知，在linux kernel中，进程与线程都是用`struct task_strust` 表示，唯一不同是 `struct task_strust` 指向资源是否共享。

### 进程1和进程0

* 测试环境：

ubuntu 10.04 ( linux 2.6.32 )

ubuntu 20.04 ( linux 5.4.0)

* 测试：

```bash
$ uname -r
2.6.32-21-generic
$ ls -l /sbin/init
-rwxr-xr-x 1 root root 125704 2010-04-02 08:13 /sbin/init
$ cat /proc/1/status
Name:	init
Tgid:	1
Pid:	1
PPid:	0
...

vernon@vernon-VB:/proc$ pstree
init─┬─NetworkManager─┬─dhclient
     │                └─{NetworkManager}
     ├─acpid
     ├─atd
     ├...

#####################################################

$ uname -r
5.4.0-47-generic
$ ls -l /sbin/init 
lrwxrwxrwx 1 root root 20 7月   7 05:38 /sbin/init -> /lib/systemd/systemd
$ cat /proc/1/status 
Name:	systemd
Tgid:	1
Pid:	1
PPid:	0
...
$ pstree
systemd─┬─ModemManager───2*[{ModemManager}]
        ├─NetworkManager───2*[{NetworkManager}]
        ├─accounts-daemon───2*[{accounts-daemon}]
        ├─acpid
        ├...
```

* 总结：

`init`进程是整个系统的第一个进程，也就是1号进程。

1号进程有父进程 0号进程，即 `idle`进程

### 孤儿进程

* 测试环境：

ubuntu 10.04 ( linux 2.6.32 )

ubuntu 20.04 ( linux 5.4.0)

* 测试：

```bash
$ cat life_period.c 
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>

int main(void)
{
	pid_t pid,wait_pid;
	int status;

	pid = fork();

	if (pid==-1)	{
		perror("Cannot create new process");
		exit(1);
	} else 	if (pid==0) {
		printf("child process id: %ld\n", (long) getpid());
		pause();
		_exit(0);
	} else {
		printf("parent process id: %ld\n", (long) getpid());
		wait_pid=waitpid(pid, &status, WUNTRACED | WCONTINUED);
		if (wait_pid == -1) {
			perror("cannot using waitpid function");
			exit(1);
		}

		if(WIFSIGNALED(status))
			printf("child process is killed by signal %d\n", WTERMSIG(status));

		exit(0);
	}
}

#####################################################

$ uname -r
2.6.32-21-generic
$ gcc life_period.c
$ ./a.out
parent process id: 1795
child process id: 1796
$ pstree
init─┬─NetworkManager─┬─dhclient
     │                └─{NetworkManager}
     ├─acpid
     |.......
     ├─gnome-terminal─┬─bash───a.out───a.out
     │                ├─2*[bash]
     │                ├─gnome-pty-helpe
     │                └─{gnome-terminal}
     ├─gvfs-afc-volume───{gvfs-afc-volum}

$ kill 1795
$ pstree
init─┬─NetworkManager─┬─dhclient
     │                └─{NetworkManager}
     ├─a.out
     ├─acpid
     |.......
     ├─gnome-terminal─┬─3*[bash]
     │                ├─gnome-pty-helpe
     │                └─{gnome-terminal}
     ├─gvfs-afc-volume───{gvfs-afc-volum}

#####################################################

$ uname -r
5.4.0-47-generic
$ ./a.out 
parent process id: 6323
child process id: 6324
$ pstree
systemd─┬─ModemManager───2*[{ModemManager}]
        ├....
        ├─systemd─┬─(sd-pam)
        │         ├......
        │         ├─gnome-terminal-─┬─bash───a.out───a.out
        │         │                 ├─bash───pstree
        │         │                 └─5*[{gnome-terminal-}]
$ kill 6323
$ pstree
systemd─┬─ModemManager───2*[{ModemManager}]
        ├....
        ├─systemd─┬─(sd-pam)
        │         ├......
        │         ├─a.out
        │         ├─gnome-terminal-─┬─2*[bash]
        │         │                 ├─bash───pstree
        │         │                 └─5*[{gnome-terminal-}]
```

* 总结：

当子进程先死亡时，父进程负责得到子进程死亡原因以及清除`struct task_strust` 

当父进程先死亡时，子进程变成孤儿进程，此时linux系统需要给子进程找一个养父进程。在linux 3.4之前，养父进程一般都是init进程；在linux 3.4之后，养父进程一般都是`SUBREAPER`标志的进程，如上图 每一个用户的systemd进程就是用`SUBREAPER`标志的进程。

### 进程睡眠

* 测试环境：

宋宝华老师的驱动开发书 －－ globalfifo.c

* 测试：

```c
static ssize_t globalfifo_write(struct file *filp, const char __user *buf,
					size_t count, loff_t *ppos)
{
    DECLARE_WAITQUEUE(wait, current);

    add_wait_queue(&dev->w_wait, &wait);
    ...
    __set_current_state(TASK_INTERRUPTIBLE);
    schedule();
    ...
}
```

* 总结：

进程进入睡眠状态，可分为两种：深度睡眠与浅度睡眠。

深度睡眠，即`UNINTERRUPTIBLE_SLEEP`，只有当**所需资源**到达时，进程才被唤醒。

浅度睡眠，即`INTERRUPTIBLE_SLEEP`，当**所需资源或信号**到达时，进程才被唤醒。

进程进入睡眠状态，即调用`add_wait_queue()`将进程放入等待队列中，设置为`__set_current_state(UNINTERRUPTIBLE_SLEEP)`或`__set_current_state(INTERRUPTIBLE_SLEEP)`后，调用`schedule()`。此时进程放弃CPU，CPU调度另一个进程继续执行。当**所需资源或信号**到达时，通过调用`wake_up()`唤醒进入睡眠状态的进程，从`schedule()`下一行源码继续执行。

