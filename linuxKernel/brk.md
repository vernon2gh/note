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

## 详细函数调用关系

```c
SYSCALL_DEFINE1(brk
    // shrinking brk
    mas_set()
    mas_find()
    do_brk_munmap()

    // increase brk
    mas_set()
    mas_find()
    mas_prev()
    do_brk_flags()
```

应用层调用系统调用 brk()，分为两种情况，收缩与增加 brk。
其中第一次设置 brk 属于增加 brk 的一种特殊情况。

当收缩 brk 时，通过 mas_find() 找到 brk 对应的 VMA，
然后调用 do_brk_munmap() 对需要收缩的那部分 VMA 进行 unmap

当增加 brk 时，通过 mas_find() 检查新 brk 值是否在 stack_guard_gap 区域中。
如果是，越界，直接退出。
否则，代表能够增加 brk，调用 mas_prev() 找到 brk 对应的 VMA，
然后调用 do_brk_flags() 进行增加 brk 的一系列操作

## 详细函数解析

```c
do_brk_munmap()
    do_mas_align_munmap(..., vma, start, end, ...)
        __split_vma()
        munmap_sidetree()
        unmap_region()
            unmap_vmas()
            free_pgtables()
        mas_set()
        remove_mt()
        __mt_destroy()
```

vma 代表需要 unmap 的第一个 VMA，
start 代表需要 unmap 区域的起始地址，
end 代表需要 unmap 区域的结束地址。

如果 start 大于 `vma->vm_start`，代表第一个 VMA 只有部分虚拟地址区域需要进行 unmap，
所以需要调用 __split_vma() 将第一个 VMA 以 start 为中间值，划分为两个 VMA
`[vma->vm_start, start)` `[start, vma->vm_end)`，同时将 vma 重新赋值为需要 unmap 的虚拟区域
对应的VMA `[start, vma->vm_end)`，因为 vma 代表需要 unmap 的第一个 VMA。

通过 mas_for_each() 循环遍历 `[start, end]` 区域内的所有 VMA，将其加入 mas_detach 中。

如果最后一个 VMA 的 `vma->vm_end` 大于 end，代表最后一个 VMA 只有部分虚拟地址区域需要进行 unmap，
所以需要调用 __split_vma() 将最后一个 VMA 以 end 为中间值，划分为两个 VMA
`[vma->vm_start, end)` `[end, vma->vm_end)`，最后将新 VMA `[vma->vm_start, end)`
加入 mas_detach 中即可。

接着调用 mas_set_range(mas, start, end - 1) 与 mas_store_prealloc(mas, NULL)，
将 mas 在 `[start, end - 1]` 范围内的 entry 设置为 NULL

最后，将执行真正的 unmap 一系列操作，如下：

1. 调用 unmap_vmas()，将 VMA 进行取消映射，释放之前映射到 VMA 对应物理内存到 buddy 分配器
2. 调用 free_pgtables()，将 VMA 对应页表本身进行取消映射，释放页表对应的物理内存到 buddy 分配器
3. 调用 remove_mt()，释放 VMA 本身对应的物理内存到 slab 分配器
4. 调用 __mt_destroy()，释放 maple tree node 对应的物理内存到 slab 分配器

```c
do_brk_flags()
    // expand the existing vma
    mas_set_range()
    mas_preallocate()
    mas_store_prealloc()
    // first setting up brk
    vm_area_alloc()
    mas_set_range()
    mas_store_gfp()
```

首先判断是否是第一次设置 brk？
如果不是，扩展存在的 VMA，直接更新 vma->vm_end = 新 brk 值，同时调用 mas_store_prealloc() 更新 VMA
在 mas 中的范围为 `[vma->vm_start, vma->vm_end - 1]`。
如果是，调用 vm_area_alloc() 向 slab 分配器申请新 VMA，初始化 vma->vm_start、vma->vm_end 等，
然后调用 mas_store_gfp() 将新 VMA 存储在 mas `[vma->vm_start, vma->vm_end - 1]` 范围中
