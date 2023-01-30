## 概述

mmap() 可以分为四大类，如下：

* 私有匿名映射
* 共享匿名映射
* 私有文件映射
* 共享文件映射

下面以 私有匿名映射 为例子，进行讲解

函数原型:

```c
#include <sys/mman.h>

void *mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset);
int munmap(void *addr, size_t length);
```

例子：

```c
#include <stdio.h>
#include <sys/mman.h>

int main(int argc, char *argv[])
{
	char *buf = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
	*buf = 0x11;
	printf("buf 0x%x\n", *buf);

	munmap(buf, 4096);

	return 0;
}
```

调用流程：用户空间接口 ~ 系统调用接口

```bash
$ strace ./a.out
mmap(NULL, 4096, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS, -1, 0) = 0x7f0b1599a000
write(1, "buf 0x11\n", 9buf 0x11
)               = 9
munmap(0x7f0b1599a000, 4096)            = 0
```

调用流程：系统调用接口 ~ 内核空间

```c
/* user space ->    kernel space           : file */
mmap()        -> SYSCALL_DEFINE6(mmap      : arch/x86/kernel/sys_x86_64.c
                       |                     arch/arm64/kernel/sys.c
                 ksys_mmap_pgoff()         : mm/mmap.c
munmap()      -> SYSCALL_DEFINE2(munmap    : mm/mmap.c
```

调用流程：内核空间

```c
SYSCALL_DEFINE6(mmap
    ksys_mmap_pgoff()
        vm_mmap_pgoff()
            do_mmap()
                get_unmapped_area()
                    arch_get_unmapped_area_topdown()
                        vm_unmapped_area()
                            unmapped_area_topdown()
                mmap_region()
                    vma_expand() -> return
                    vm_area_alloc()
                    mas_preallocate()
                    vma_mas_store()

SYSCALL_DEFINE2(munmap
    __vm_munmap()
        do_mas_munmap()
            mas_find()
            do_mas_align_munmap()
```

do_mmap() 调用 get_unmapped_area() 查找空闲的虚拟地址空间，
mmap_region() 将空闲的虚拟地址空间扩展到旧 VMA 或 创建新 VMA 并且加入到 maple tree 中

munmap(addr) 从 maple tree 中以 addr 为 index 找到对应的 vma，
然后调用 do_mas_align_munmap() 将 vma 进行 unmap

## 零散知识点

1. `calc_vm_prot_bits()`/`alc_vm_flag_bits()` 将 mmap prot (PROT_XXX) 权限,
mmap flags (MAP_XXX) 标志转换成 vm_flags (VM_XXX) 标志，保存在 `vma->vm_flags`

2. `vm_get_page_prot(vm_flags)` 得到 PTE 真实权限，保存在 `vma->vm_page_prot`
（与体系架构相关）

3. 在 `do_page_fault()` 获得 `Page Fault` 的类型，然后在 `__do_page_fault()`
将 `Page Fault` 的类型 与 `vma->vm_flags` 进行对比，如果不一致，说明权限不足，退出。
比如：`buf = mmap(READ_ONLY)`，但是对 buf 进行写操作

4. 当 mmap flags 没有 MAP_FIXED 标志时，指定的虚拟地址 vaddr 有最低值要求，必须大于等于4096。
为什么？因为 Linux 默认将虚拟地址 NULL 当成 panic，所以不能对虚拟地址 NULL 进行映射。
又因为映射的虚拟地址必须4096对齐，所以虚拟地址必须大于等于4096

5. 一个进程的 VMA 个数必须小于 sysctl_max_map_count。
为什么？ ELF 格式的 section 个数限制，最大值为 unsigned short。
详细解释：`include/linux/mm.h` 的 179 ~ 196 行

6. mmap len 必须小于 TASK_SIZE，即 小于最大的进程虚拟地址

7. mm.mmap_base : 虚拟地址空间中用于映射的起始地址
