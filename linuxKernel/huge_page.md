## 简述

huge page种类：

1. 静态大页(persistent hugepage)，通过用户自行控制它的分配、释放、使用

2. 透明大页(transparent hugepage)，由系统自己控制透明大页的分配、释放、使用。
若用户开启透明大页功能，系统会在后台运行一个khugepaged的内核线程扫描系统内存，
将合适的内存合并成为大页，用户无感

huge page优点：

1. 降低TLB miss的概率。
因为一个页表项能够覆盖更多的内存，TLB cache大小有限，并且程序访问地址局部性原理

2. 降低 walk page table的次数。
当 huge page 为 2MB时，walk时会减少一级页表

对外体现为内存访问带宽提高了

## 静态大页(persistent hugepage)

### 如何预分配huge page的页数/大小？

1. bootargs 参数

> Documentation/admin-guide/kernel-parameters.txt

* hugepagesz= 指定预分配huge page的大小
* hugepages= 指定预分配huge page的页数

2. 通过/sys/kernel/mm/hugepages/配置

> Documentation/ABI/testing/sysfs-kernel-mm-hugepages
>
> Documentation/admin-guide/mm/hugetlbpage.rst

`$ echo <pages> > /sys/kernel/mm/hugepages/hugepages-xxx/nr_hugepages`

指定预分配huge page的大小为 xxxx，页数为 pages。比如：

```bash
$ ls /sys/kernel/mm/hugepages/hugepages-2048kB/
demote                   nr_hugepages             resv_hugepages
demote_size              nr_hugepages_mempolicy   surplus_hugepages
free_hugepages           nr_overcommit_hugepages
```

* nr_hugepages，目前支持的huge page页数，可动态配置
* free_hugepages，空闲huge page页数
* resv_hugepages, 通过mmap()申请虚拟地址空间成功，但是还没有申请物理内存的huge page页数
* surplus_hugepages，申请的huge page页数 减去 支持的huge page页数

### 如何申请从huge page中分配内存？

1. 调用mmap()时添加MAP_HUGETLB标志，比如：

```c
#include <sys/mman.h>

char *vaddr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
```

## 透明大页(transparent hugepage)

### 如何预分配huge page的页数/大小？如何申请从huge page中分配内存？

> Documentation/admin-guide/mm/transhuge.rst

```bash
$ ls /sys/kernel/mm/transparent_hugepage/
defrag          hpage_pmd_size  shmem_enabled
enabled         khugepaged      use_zero_page
$ ls /sys/kernel/mm/transparent_hugepage/khugepaged/
alloc_sleep_millisecs  max_ptes_none          pages_collapsed
defrag                 max_ptes_shared        pages_to_scan
full_scans             max_ptes_swap          scan_sleep_millisecs
```

## 源码解析

> base linux v5.19-rc6

> huge page 与 gigantic page 区别：
>
> 页面阶小于MAX_ORDER称为huge page，大于等于MAX_ORDER称为gigantic page，
> 比如：2MB是huge page，1GB是gigantic page

比如: 通过文件系统配置huge page个数

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
当pages比目前支持的huge page页数多，调用alloc_pool_huge_page()逐个申请huge page加入huge page内存池；
当pages比目前支持的huge page页数少，调用remove_pool_huge_page()逐个从huge page内存池中删除多余的huge page

```c
alloc_pool_huge_page
    alloc_fresh_huge_page
    put_page
```

循环从不同node调用alloc_fresh_huge_page()分配一页huge page，
如果分配一页huge page成功，将调用put_page()将此huge page从page分配器中删除，
并且加入到huge page内存池中（nr_hugepages++）

alloc_fresh_huge_page()，调用hstate_is_gigantic()判断想要分配的内存是否属于gigantic page？
如果是，调用alloc_gigantic_page()，如果支持从CMA分配内存，直接从CMA分配内存; 否则，从page分配器（buddy算法）中分配内存。
否则属于huge page，调用alloc_buddy_huge_page()，直接从page分配器（buddy算法）中分配内存。

```c
remove_pool_huge_page
    page = list_entry
    remove_hugetlb_page
```

循环从不同node调用list_entry()获得空闲一页huge page，
然后调用remove_hugetlb_page()将此页huge page从huge page内存池中删除（nr_hugepages--），
最后将此页huge page返回，返回后将huge page加入page_list链表中。

```c
update_and_free_pages_bulk(page_list)
    update_and_free_page
        __update_and_free_page
flush_free_hpage_work
```

update_and_free_pages_bulk()最终调用__update_and_free_page()做真正的释放内存操作
