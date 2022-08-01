## 简介

Linux 支持的共享内存方式：System V 和 POSIX 共享内存。

## POSIX 共享内存

函数原型:

```c
int shm_open(const char *name, int oflag, mode_t mode);
int shm_unlink(const char *name);
```

调用 `shm_open()` 创建或打开一个共享内存对象 `/dev/shm/<name>`，属性为 `oflag，`，
文件节点的权限为 `mode`（当 `oflag` 有 `O_CREAT` 标志，创建一个新共享内存对象）

一个新共享内存对象被创建时，初始长度为 0，需要调用 `ftruncate()` 来设置
共享内存对象的大小，并且自动初始化为 0。

正常调用 `mmap()/munmap` 将共享内存对象映射到进程虚拟地址空间中/从进程虚拟地址空间中解除映射即可。

共享内存对象需要手动调用 `shm_unlink()` 进行删除，否则一直存在系统中或系统重启才消失

可以在命令行执行 `ls /dev/shm/` 查看目前系统中存在的共享内存对象

例子：

```c
#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

int main(void)
{
	struct stat stat_buf;
	char *buf;
	int fd;

	fd = shm_open("shmtest", O_RDWR | O_CREAT, 0777);

	ftruncate(fd, 4096);

	fstat(fd, &stat_buf);
	printf("st_size 0x%lx\n", stat_buf.st_size);

	buf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	*buf = 0x11;
	printf("buf 0x%x\n", *buf);

	munmap(buf, 4096);

	shm_unlink("shmtest");

	return 0;
}
```

## System V 共享内存

函数原型:

```c
int shmget(key_t key, size_t size, int shmflg);
void *shmat(int shmid, const void *shmaddr, int shmflg);
int shmdt(const void *shmaddr);
int shmctl(int shmid, int cmd, struct shmid_ds *buf);
```

调用 `shmget()` 通过 `key` 创建或打开一个共享内存对象，大小为 `size`，
文件节点的权限为 `shmflg`（当 `shmflg` 有 `IPC_CREAT` 标志，创建一个新共享内存对象）

调用 `shmat()` 将 `shmid` 对应的共享内存对象 映射到 进程虚拟地址空间中，
如果 `shmaddr` 为 NULL，由内核自动查找空闲的虚拟地址空间进行映射，并且返回。

调用 `shmdt()` 将虚拟地址 `shmaddr` 对应的共享内存对象进行解除映射

共享内存对象需要手动调用 `shmctl()` 进行删除，否则一直存在系统中或系统重启才消失

可以在命令行执行 `ipcs -m` 查看目前系统中存在的共享内存对象

例子：

```c
#include <stdio.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int main(void)
{
        int shmid;
        char *buf;

	printf("%s: 0x%x\n", __func__, ftok("filename", 0));

	shmid = shmget(0x11, 2048, IPC_CREAT|0666);

	buf = shmat(shmid, NULL, 0);
	*buf = 0x12;
	printf("buf 0x%x\n", *buf);
	shmdt(buf);

	shmctl(shmid, IPC_RMID, NULL);

	return 0;
}
```
