## 概述

brk 全称 the location of the program break, 即 `heap top`

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

调用流程：内核空间

```c
SYSCALL_DEFINE1(brk
    // shrinking brk
    mas_set()
    mas_find()
    do_brk_munmap()
        do_mas_align_munmap()

    // increase brk
    mas_set()
    mas_find()
    mas_prev()
    do_brk_flags()
```

应用层调用系统调用 brk()，分为两种情况，收缩与增加 brk。
其中第一次设置 brk 属于增加 brk 的一种特殊情况。

当收缩 brk 时，通过 mas_find() 找到 brk 对应的 VMA，
然后调用 [do_mas_align_munmap()](../function_parse/do_mas_align_munmap.md)
对需要收缩的那部分 VMA 进行 unmap

当增加 brk 时，通过 mas_find() 检查新 brk 值是否在 stack_guard_gap 区域中。
如果是，越界，直接退出。
否则，代表能够增加 brk，调用 mas_prev() 找到 brk 对应的 VMA，
然后调用 [do_brk_flags()](../function_parse/do_brk_flags.md)
进行增加 brk 的一系列操作
