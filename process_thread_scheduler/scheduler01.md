#### 1. linux进程周期

就绪、运行、睡眠、停止、僵死

#### 2. task_struct

```c
struct task_struct {
	...
    pid_t pid;
    struct mm_struct *mm;
    struct fs_struct *fs;
    struct files_struct *files;
    ...
};

struct mm_struct {
    ...
    struct vm_area_struct * mmap;
    pgd_t * pgd;
    ...
};

struct fs_struct {
    ...
    struct path root, pwd;
    ...
};

struct files_struct {
    ...
    struct fdtable fdtab;
    struct file * fd_array[NR_OPEN_DEFAULT];
    ...
};
```

task_struct形成 链表、树、哈希 来管理

#### 2. 查看目前系统支持最多pid数量

```bash
$ cat /proc/sys/kernel/pid_max
32768
```

#### 3. 内存泄漏

僵死进程，资源已经释放，无内存泄漏，只存下 task_struct 供父进程查死亡原因

进程运行中，运行越久，内存消耗越多，即 发生内存泄漏

#### 4. 停止/恢复进程与作业控制  cpulimit

ctrl + z : 暂停执行进程

fg/bg    : 恢复进程执行



```bash
$ cpulimit -l 20 -p 10111
```

限制pid为10111进程的cpu使用率不超过20%

#### 5. fork

`$ vim fork1.c `

```c
main()
{
	fork();
	printf("hello\n");
	fork();	
	printf("hello\n");
	while(1);
}
```

`$ vim fork2.c `

```c
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
		printf("a\n");
	} else {
		printf("b\n");
	}

	printf("c\n");
	while(1);
}
```

`$ vim life_period.c `

```c
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
#if 0 /* define 1 to make child process always a zomie */
		printf("ppid:%d\n", getpid());
		while(1);
#endif
		do {
			wait_pid=waitpid(pid, &status, WUNTRACED | WCONTINUED);

			if (wait_pid == -1) {
				perror("cannot using waitpid function");
				exit(1);
			}

			if (WIFEXITED(status))
				printf("child process exites, status=%d\n", WEXITSTATUS(status));

			if(WIFSIGNALED(status))
				printf("child process is killed by signal %d\n", WTERMSIG(status));

			if (WIFSTOPPED(status))
				printf("child process is stopped by signal %d\n", WSTOPSIG(status));

			if (WIFCONTINUED(status))
				printf("child process resume running....\n");

		} while (!WIFEXITED(status) && !WIFSIGNALED(status));

		exit(0);
}
```

