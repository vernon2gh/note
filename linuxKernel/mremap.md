## 概述

mremap() 能够扩展/收缩一个存在的虚拟内存区域 VMA，
如果 VMA 前后没有空闲虚拟内存区域进行扩展，可能会将 VMA 进行移动。

函数原型:

```c
#define _GNU_SOURCE
#include <sys/mman.h>

void *mremap(void *old_address, size_t old_size, size_t new_size, int flags,
            ... /* void *new_address */);
```

例子：

```c
#define _GNU_SOURCE
#include <stdio.h>
#include <sys/mman.h>

#define BUF_SIZE        4096

int main(int argc, char *argv[])
{
    char *buf;

    buf = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    buf[0] = 0x11;
    printf("buf[0] 0x%x\n", buf[0]);

    buf = mremap(buf, BUF_SIZE, BUF_SIZE * 2, MREMAP_MAYMOVE);
    if (buf == MAP_FAILED) {
        printf("MAP FAILED\n");
        return -1;
    }

    buf[4096] = 0x12;
    printf("buf[4096] 0x%x\n", buf[4096]);

    munmap(buf, BUF_SIZE * 2);

    return 0;
}
```

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f2067903000
write(1, "buf[0] 0x11\n", 12buf[0] 0x11)           = 12
mremap(0x7f2067903000, 4096, 8192, MREMAP_MAYMOVE) = 0x7f20678c8000
write(1, "buf[4096] 0x12\n", 15buf[4096] 0x12)     = 15
munmap(0x7f20678c8000, 8192)                       = 0
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space ->    kernel space           : file */
mremap()      -> SYSCALL_DEFINE5(mremap    : mm/mremap.c
```

调用流程：内核空间

```c
SYSCALL_DEFINE5(mremap
    do_vmi_munmap()
    vma_merge()
    get_unmapped_area()
    move_vma()
```

如果 old_len >= new_len，代表需要收缩 VMA，直接调用 do_vmi_munmap()
将 VMA 不需要的那部分进行 unmap

如果 old_len == vma->vm_end - addr 并且此 VMA 后面还有足够的空闲虚拟内存区域，
代表可以直接合并到前/后 VMA, 调用 [vma_merge()](../function_parse/vma_merge.md)
将新空闲虚拟内存区域合并到前/后 VMA

否则，代表只能从后面空闲虚拟内存区域来查找分配一个新 VMA，并且做移动数据操作。于是，
调用 get_unmapped_area() 从 maple tree 获得第一个空闲虚拟内存区域的起始地址，
然后调用 move_vma() 申请新 VMA（以第一个空闲虚拟内存区域的起始地址为开始），
将旧 VMA 所有数据移动到新 VMA，同时将新 VMA 加入 maple tree 中

```c
move_vma()
    copy_vma()
    move_page_tables()

    if flags contain MREMAP_DONTUNMAP
        return new_addr;
    else
        do_vmi_munmap()
        return new_addr;
```

调用 copy_vma() 初始化新 VMA，如下：

调用 vm_area_dup() 从 vm_area_cachep 分配新 VMA，然后以旧 VMA 为基础进行初始化新 VMA，
最后调用 vma_link() 将新 VMA 存储到 maple tree 中

调用 move_page_tables() 将页表在 [old_addr, old_addr + old_len) 范围内的 PTE entry
移动到 [new_addr, new_addr + old_len) 中，如下：

1. 如果 extent 是 PUD_SIZE，在 PUD level 进行移动页表，如果成功，continue，否则，
2. 如果 extent 是 PMD_SIZE，在 PMD level 进行移动页表，如果成功，continue，否则，
3. 以 PAGE_SIZE 为单位，在 PTE level 逐一将每一页进行移动

如果有 MREMAP_DONTUNMAP 标志，代表旧 VMA 不进行 unmap，当又满足某些条件时，
也只是简单调用 unlink_anon_vmas() 取消旧 VMA 的匿名反向映射关系。
否则，调用 do_vmi_munmap() 将旧 VMA 进行 unmap。
最后返回 new_addr
