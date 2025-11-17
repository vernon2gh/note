## 简述

hugepage 种类：

1. 静态大页(persistent hugepage)，也叫 hugetlb hugepage, 由 hugepage pool 实现
管理，用户空间需要手动分配和释放。

2. 透明大页(transparent hugepage)，由系统自动控制分配和释放。
系统还会在后台运行一个khugepaged 内核线程扫描系统内存，将合适的内存合并成为大页，
用户无感。

hugepage 优点：

1. 降低TLB miss的概率。
因为一个页表项能够覆盖更多的内存，TLB cache大小有限，并且程序访问地址局部性原理

2. 降低 walk page table的次数。
当 hugepage 为 2MB时，walk时会减少一级页表

对外体现为内存访问带宽提高了

## 静态大页(persistent hugepage)

### 每一个 hugepage 的大小

* PMD-sized 2MB
* PUD-sized 1GB
* cont-pte 2MB / cont-pmd 1GB

### 如何预分配 hugepage 的页数/大小？

> Documentation/admin-guide/kernel-parameters.txt
>
> Documentation/ABI/testing/sysfs-kernel-mm-hugepages
>
> Documentation/admin-guide/mm/hugetlbpage.rst

通过 `bootargs` 参数进行配置，如下：

* `hugepagesz=` 指定预分配 hugepage 的大小
* `hugepages=` 指定预分配 hugepage 的页数

或者通过 `/sys/kernel/mm/hugepages/` 进行配置，如下：

`$ echo <pages> > /sys/kernel/mm/hugepages/hugepages-xxx/nr_hugepages`

指定预分配 hugepage 页数为 pages，并且每一个 hugepage 的大小为 xxxx。

### 如何申请从 hugepage 中分配内存？

调用mmap()时添加 MAP_HUGETLB 标志，比如：

```c
#include <sys/mman.h>

char *vaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
```

### /proc/meminfo 的相关字段解释

> Documentation/admin-guide/mm/hugetlbpage.rst

* HugePages_Total, kernel's hugepage pool中所有 hugepage 总和
* HugePages_Free,  kernel's hugepage pool中空闲 hugepage 的总和
* HugePages_Rsvd,  通过 mmap() 申请虚拟地址空间成功，但是还没有申请物理内存的 hugepage 页数
* HugePages_Surp,  申请的 hugepage 页数减去所有 hugepage 页数
* Hugepagesize,    默认每一个 hugepage 的大小，如：2MB/1GB

## 透明大页(transparent hugepage)

### 每一个 hugepage 的大小

* PMD-sized 2MB
* PUD-sized 1GB

### 如何分配 hugepage？

> Documentation/admin-guide/mm/transhuge.rst

`$ echo xxx > /sys/kernel/mm/transparent_hugepage/enabled`

当 `enabled = always` 时，在 pagefault 自动判断是否为 hugepage，如果是，直接
分配映射 2MB 内存，否则，只分配映射 4KB 内存。

当 `enabled = madvise` 时，只有使用 `madvise(MADV_HUGEPAGE)`，才会在 pagefault
分配映射 2MB 内存，否则，只分配映射 4KB 内存。

### /proc/meminfo 的相关字段解释

> Documentation/filesystems/proc.rst

* AnonHugePages
* ShmemHugePages
* ShmemPmdMapped
* FileHugePages
* FilePmdMapped

## 源码解析

### 静态大页(persistent hugepage)

> base linux v5.19-rc6

> huge page 与 gigantic page 区别：
>
> 页面阶小于MAX_ORDER称为 huge page，大于等于MAX_ORDER称为gigantic page，
> 比如：2MB是 huge page，1GB 是 gigantic page

比如: 通过文件系统配置 hugepage 个数

```c
## kernel/sysctl.c

static struct ctl_table vm_table[] = {
	{
	.procname	= "nr_hugepages",
	.data		= NULL,
	.maxlen		= sizeof(unsigned long),
	.mode		= 0644,
	.proc_handler	= hugetlb_sysctl_handler,
	},
}
```

当对`/sys/kernel/mm/hugepages/hugepages-xxx/nr_hugepages`进行写操作时，
调用hugetlb_sysctl_handler()

```c
## mm/hugetlb.c

hugetlb_sysctl_handler
    hugetlb_sysctl_handler_common
        __nr_hugepages_store_common
            set_max_huge_pages
                alloc_pool_huge_page

                remove_pool_huge_page
                update_and_free_pages_bulk
                flush_free_hpage_work
```

执行 echo <pages> > nr_hugepages,
当pages比目前支持的 hugepage 页数多，调用alloc_pool_huge_page()逐个申请 hugepage 加入 hugepage 内存池；
当pages比目前支持的 hugepage 页数少，调用remove_pool_huge_page()逐个从 hugepage 内存池中删除多余的 hugepage

```c
alloc_pool_huge_page
    alloc_fresh_huge_page
    put_page
```

循环从不同node调用alloc_fresh_huge_page()分配一页 hugepage，
如果分配一页 hugepage 成功，将调用put_page()将此 hugepage 从page分配器中删除，
并且加入到 hugepage 内存池中（nr_hugepages++）

alloc_fresh_huge_page()，调用hstate_is_gigantic()判断想要分配的内存是否属于gigantic page？
如果是，调用alloc_gigantic_page()，如果支持从CMA分配内存，直接从CMA分配内存; 否则，从page分配器（buddy算法）中分配内存。
否则属于 hugepage，调用alloc_buddy_huge_page()，直接从page分配器（buddy算法）中分配内存。

```c
remove_pool_huge_page
    page = list_entry
    remove_hugetlb_page
```

循环从不同node调用list_entry()获得空闲一页 hugepage，
然后调用remove_hugetlb_page()将此页 hugepage 从 hugepage 内存池中删除（nr_hugepages--），
最后将此页 hugepage 返回，返回后将 hugepage 加入page_list链表中。

```c
update_and_free_pages_bulk(page_list)
    update_and_free_page
        __update_and_free_page
flush_free_hpage_work
```

update_and_free_pages_bulk()最终调用__update_and_free_page()做真正的释放内存操作

### 透明大页(transparent hugepage)

直接通过 `mmap(start_vaddr_align_2MB, size_2MB, MAP_PRIVATE | MAP_ANONYMOUS)`
分配 2MB 虚拟内存，第一次触发 write pagefault 时，进行分配映射 2MB 物理内存，如下：

```c
handle_mm_fault()
    __handle_mm_fault()
        create_huge_pmd()
            do_huge_pmd_anonymous_page()
            |    transhuge_vma_suitable()
            |    vma_alloc_folio()
            |    __do_huge_pmd_anonymous_page()
            |    |    mk_huge_pmd()
            |    |    maybe_pmd_mkwrite()
            |    |    set_pmd_at()
```

直接通过 `mmap(start_vaddr_align_2MB, size_2MB, MAP_PRIVATE | MAP_ANONYMOUS)`
分配 2MB 虚拟内存，第一次触发 read pagefault 时，进行映射 2MB 零页物理内存，如下：

```c
handle_mm_fault()
    __handle_mm_fault()
        create_huge_pmd()
            do_huge_pmd_anonymous_page()
            |    transhuge_vma_suitable()
            |    mm_get_huge_zero_page()
            |    set_huge_zero_page()
            |    |    mk_pmd()
            |    |    pmd_mkhuge()
            |    |    set_pmd_at()
```

当映射 2MB 零页物理内存后，再触发 write pagefault 时，首先拆分 PMD 成多个 PTE，
这些 PTE 都映射 4KB 零页物理内存，然后按照正常触发 write 4KB zero-page pagefault，
调用 `do_wp_page()` 进行分配映射 4KB 物理内存，如下：

```c
handle_mm_fault()
    __handle_mm_fault()
        wp_huge_pmd()
        |    do_huge_pmd_wp_page()
        |        __split_huge_pmd()
        |            __split_huge_pmd_locked()
        |                __split_huge_zero_page_pmd()
        |                |    pfn_pte(my_zero_pfn())  // 4KB zero-page
        |                |    pte_mkspecial()
        |                |    set_pte_at()
        handle_pte_fault()
            do_wp_page()
```


wakeup khugepaged 的场景

- pagefault 直接分配2MB，同时唤醒khugepaged
- `madvise(MADV_HUGEPAGE)` 直接唤醒khugepaged
- `mmap()`/`mprotect()`/`mremap()` 导致vma合并时唤醒khugepaged

```
pagefault handle_mm_fault()     -----+
madvise   hugepage_madvise()         +--> 唤醒khugepaged的核心函数
vma       vma_merge_existing_range() +--> khugepaged_enter_vma()
vma       vma_merge_new_range() -----+
```
