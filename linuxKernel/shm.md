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

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
statfs("/dev/shm/", {f_type=TMPFS_MAGIC, f_bsize=4096, f_blocks=1018431, f_bfree=1018431, f_bavail=1018431, f_files=1018431, f_ffree=1018430, f_fsid={val=[0, 0]}, f_namelen=255, f_frsize=4096, f_flags=ST_VALID|ST_NOSUID|ST_NODEV|ST_NOATIME}) = 0
futex(0x7f3150c48390, FUTEX_WAKE_PRIVATE, 2147483647) = 0
openat(AT_FDCWD, "/dev/shm/shmtest", O_RDWR|O_CREAT|O_NOFOLLOW|O_CLOEXEC, 0777) = 3
ftruncate(3, 4096)                      = 0
fstat(3, {st_mode=S_IFREG|0755, st_size=4096, ...}) = 0
write(1, "st_size 0x1000\n", 15st_size 0x1000
)        = 15
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, 3, 0) = 0x7f3150e81000
write(1, "buf 0x11\n", 9buf 0x11
)               = 9
munmap(0x7f3150e81000, 4096)            = 0
unlink("/dev/shm/shmtest")              = 0
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space ->    kernel space           : file */
shm_open()    -> SYSCALL_DEFINE4(openat    : fs/open.c
shm_unlink()  -> SYSCALL_DEFINE1(unlink    : fs/namei.c

ftruncate()   -> SYSCALL_DEFINE2(ftruncate : fs/open.c
fstat()       -> SYSCALL_DEFINE2(fstat     : fs/stat.c

mmap()        -> SYSCALL_DEFINE6(mmap      : arch/x86/kernel/sys_x86_64.c
                       |                     arch/arm64/kernel/sys.c
                 ksys_mmap_pgoff()         : mm/mmap.c
munmap()      -> SYSCALL_DEFINE2(munmap    : mm/mmap.c
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

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
stat("filename", {st_mode=S_IFREG|0644, st_size=0, ...}) = 0
write(1, "main: 0xae00\n", 13main: 0xae00
)          = 13
shmget(0x11, 4096, IPC_CREAT|0666)      = 1
shmat(1, NULL, 0)                       = 0x7fecbb13d000
write(1, "buf 0x12\n", 9buf 0x12
)               = 9
shmdt(0x7fecbb13d000)                   = 0
shmctl(1, IPC_RMID, NULL)               = 0
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space ->    kernel space          : file */
    ftok()    -> SYSCALL_DEFINE2(stat     : fs/stat.c

    shmget()  -> SYSCALL_DEFINE3(shmget   : ipc/shm.c
    shmctl()  -> SYSCALL_DEFINE3(shmctl
    shmat()   -> SYSCALL_DEFINE3(shmat
    shmdt()   -> SYSCALL_DEFINE1(shmdt
```
