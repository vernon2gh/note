## 简介

brk 全称 the location of the program break, 即 `heap top`

## 详细解释

函数原型:

```c
int brk(void *addr);
void *sbrk(intptr_t increment);
```

例子：

```c
#include <stdio.h>
#include <unistd.h>

int main(void)
{
	printf("sbrk return %p\n", sbrk(0));

	return 0;
}
```

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
brk(NULL)                               = 0x560be4ca6000
fstat(1, {st_mode=S_IFCHR|0620, st_rdev=makedev(0x88, 0x1), ...}) = 0
brk(0x560be4cc7000)                     = 0x560be4cc7000
write(1, "sbrk return 0x560be4ca6000\n", 27sbrk return 0x560be4ca6000
) = 27
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space   ->    kernel space      : file */
sbrk() -> brk() -> SYSCALL_DEFINE1(brk  : mm/mmap.c
```
