## 概述

mprotect() 设置进程虚拟内存区域 VMA 的权限，如 读/写/执行 等

函数原型:

```c
#include <sys/mman.h>

int mprotect(void *addr, size_t len, int prot);
```

例子：

```c
#include <stdio.h>
#include <sys/mman.h>

#define BUF_SIZE        8192

int main(int argc, char *argv[])
{
    char *buf;

    buf = mmap(NULL, BUF_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (mprotect(buf, 4096, PROT_READ)) {
        printf("set protection on a region of memory FAILED.\n");
        return -1;
    }

    munmap(buf, BUF_SIZE);

    return 0;
}
```

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
mmap(NULL, 8192, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f2fe7a7d000
mprotect(0x7f2fe7a7d000, 4096, PROT_READ) = 0
munmap(0x7f2fe7a7d000, 8192)            = 0
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space ->    kernel space           : file */
mprotect()    -> SYSCALL_DEFINE3(mprotect  : mm/mprotect.c
```

调用流程：内核空间

```c
SYSCALL_DEFINE3(mprotect
    do_mprotect_pkey()
        vma_iter_init()
        vma_find()
        vma_prev()
        for_each_vma_range()
            calc_vm_prot_bits()
            map_deny_write_exec()
            mprotect_fixup()
```

调用 vma_find() 查找 [start, end) 范围内的第一个 VMA，
然后调用 vma_prev() 查找此 VMA 的上一个 VMA

接着通过 for_each_vma_range() 将 [start, end) 范围内的所有 VMA 进行遍历，
分别执行以下操作：

* 调用 calc_vm_prot_bits() 从参数 @prot 获得 newflags
* 调用 map_deny_write_exec() 检查 newflags，
  比如：writable + executable 组合是不允许的情况
* 调用 mprotect_fixup() 将 @vma 进行合并或拆分操作，以及重新初始化 vma->vm_flags,
  vma->vm_page_prot, 页表对应的 PTE 权限

```c
mprotect_fixup()
    vma_merge()
    split_vma()
    vm_flags_reset()
    vma_set_page_prot()
    change_protection()
    populate_vma_page_range()
```

首先调用 [vma_merge()](../function_parse/vma_merge.md) 尝试将
(start, end, newflags, pgoff) 合并到 prev/next VMA 中，
如果合并成功，将 (start, end, newflags, pgoff) 目前所在的 VMA 赋值给变量 @vma，
如果合并失败，调用 [split_vma()](../function_parse/split_vma.md) 将 old VMA 进行拆分，
拆分后，变量 @vma 只包括 (start, end, newflags, pgoff)。
目前变量 @vma 保存 (start, end, newflags, pgoff)，接下来 @vma 进行一系列的更新操作：

* 调用 vm_flags_reset() 将 newflags 赋值给 vma->vm_flags
* 调用 vma_set_page_prot() 从 newflags 转换成 vm_page_prot，同时赋值给 vma->vm_page_prot
* 最后做真正修改 PTE entry 权限操作，调用 change_protection() 将虚拟地址区域 [start, end) 范围内的
  PTE entry 对应的 prot bit 修改成 vma->vm_page_prot

如果 old VMA 是 私有 + 只读权限 + VM_LOCKED，同时 new VMA 添加可写权限，
调用 populate_vma_page_range() 将虚拟地址区域 [start, end] 填充物理内存
