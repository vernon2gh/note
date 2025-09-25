## 一、概述

众所周知，THP 影响应用程序运行速度有两个原因：

* 单个 TLB entry 映射更大内存，从而减少 TLB misses 的数量
* 以 PMD 2MB 为例，缺页异常次数减少 512 倍

但是 THP 也存在一些问题：

* THP 只支持以 PMD 为单位进行映射
* 在小内存应用程序，明明只需要 64KB，但是 THP 直接分配 2MB，浪费内存
* 更多的 clear_page、copy_page，耗时大

为了在兼容 THP 优势的前提下，解决以上存在问题，从而引出 多颗粒度透明大页
（multi-size THP，简称：mTHP）

mTHP 是以 PTE 映射内存到页表，支持 8KB/16KB/32KB/64KB/128KB/256KB/512KB/1024KB
多颗粒度大小，同时兼容 THP PMD（2MB）的大小，于是 mTHP 支持 `[8KB, 2MB]` 范围。
减少缺页异常次数，同时利用某些硬件架构机制将多个 entry 合并成一个 TLB entry
（如：arm64 cont-bits 机制），减少 TLB missed 概率。

使用 mTHP 的优势：

* 与基础 4KB 相比，减少缺页异常次数（例如：减少 4 倍、8 倍、16 倍等）
* 与基础 4KB 相比，减少 TLB misses 概率，arm64 架构采用 cont-bits 机制，能够将
  64KB 16x entry 合并成一个 TLB entry，x86_64 自动将 32KB 8x entry 合并成一个 TLB entry。
* 与 THP 相比，减少最大 latency，因为每个 large folio 小于 PMD，需要 clear/copy 内存更少
* 与 THP 相比，由于分配内存较小，内部碎片化的代价更低
* 与 THP 相比，系统在长时间运行后比较容易分配到 large folio。

使用 mTHP 的劣处：

* 多个 size 给用户选择，如果系统管理员选择过大 size，同样存在 THP 的问题。
* 内存管理子系统的整体代码复杂度上升，需要更多的兼容代码，如 swap, mlock, mprotect, mremep等。

THP 可以看作是 mTHP 的子集，下面我们统一使用 mTHP 的叫法。

目前 mTHP 仅适用于匿名内存、共享匿名内存 shm_anon、shm_tmpfs

* mTHP anon
* mTHP shm_anon/shm_tmpfs

mTHP 通过两种方式将大页映射到进程虚报地址空间中。

* 第一次访问时触发缺页异常，直接分配 large folio。
* 内核线程 khugepaged 在后台自动将多个 page 合并为一个 large folio。

如何使用 mTHP?（以 always 为例）

```bash
$ echo always > /sys/kernel/mm/transparent_hugepage/hugepages-xxxkB/enabled
$ echo always > /sys/kernel/mm/transparent_hugepage/hugepages-xxxkB/shmem_enabled
$ mount -t tmpfs -o size=1G,huge=always tmpfs /foo
```

## 二、mTHP anon

用户空间申请物理内存，从用户空间到内核空间的整体过程：

1. 如果是匿名页，用户空间调用 malloc()（即 mmap(MAP_ANON) or brk()）分配内存
2. 如果是文件页，用户空间调用mmap(MAP_FILE) 分配内存。

两者都在第一次访问内存时，触发缺页异常pagefault来分配物理内存来映射到虚拟地址空间。

内核空间的缺页异常处理，进入 handle_mm_fault() -> __handle_mm_fault()，
如果是（pte 为空）第一次触发 pagefault 时，通过判断 vma->vm_ops == NULL 来识别
是否匿名页触发 pagefault。如果是，调用 do_anonymous_page()，否则调用 do_fault() 走文件页流程。

当用户空间调用 malloc()（即 mmap(MAP_ANON|MAP_PRIVATE)）分配内存时，会调用
vma_set_anonymous() 将 vma->vm_ops 设置为 NULL。这样我们在触发 pagefault 时，
就能够进入 do_anonymous_page() 来分配匿名页，如果是只读操作，直接调用 my_zero_pfn()
将虚报地址映射到零页，不分配实际物理内存。如果是写操作，我们才会调用
alloc_anon_folio() 来分配内存，如果 mTHP disable，只分配 4KB 页; 如果 mTHP enable，
就会调用 thp_vma_allowable_orders() 从 huge_anon_orders_always/madvise/inherit 变量获得
/sys/kernel/mm/transparent_hugepage/hugepages-xxxKB/enabled，同时调用
thp_vma_suitable_orders() 判断没有超过 vma 范围+起始地址对齐等，获得合适 order。
我们就可以根据用户想要 mTHP size 进行分配内存，比如 用户设置 hugepages-64KB/enabled
为 always，并且 64KB 没有超过 vma 范围，同时起始地址对齐到 64KB，那么最终就返回
order = 4 来分配 64KB large folio。

```bash
$ echo always > /sys/kernel/mm/transparent_hugepage/hugepages-64kB/enabled
$
static void test_mmap_syscall(void)
{
	int size = 1024 * 1024 * 1024; // 1G
	char *buf;

	buf = mmap(0, size, PROT_READ | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	memset(buf, 'A', size);

	munmap(buf, size);
}
```

以上 demo 分配 1G 匿名页，同时将 mTHP 64KB 设置为 always，通过
`perf stat -e page-faults,cache-misses,cache-references -- ./a.out` 获得
pagefault次数、cache-misses、cache-references，可知使用 mTHP 64KB 后，
pagefaul次数从 262169 下降到 16410，cache-misses 从5.62% 下降到 2.2%

## 三、mTHP shm_anon/shm_tmpfs

当用户空间调用 malloc()（即 mmap(MAP_ANON|MAP_SHARED)） 分配内存时，shm_anon
会调用 shmem_mmap() 将 vma->vm_ops 设置为 shmem_anon_vm_ops，shm_tmpfs 会调用
shmem_mmap() 将 vma->vm_ops 设置为 shmem_vm_ops。这样我们在触发 pagefault 时，
就能够进入 do_fault() 来分配共享匿名页，先在 xarray 查找 pagecache，如果找到，
直接返回。如果找不到，同时 mTHP disable，只分配 4KB 页;
mTHP enable，如果是 shm_anon，就会调用 shmem_allowable_huge_orders() 从
huge_shmem_orders_always/madvise/inherit 变量获得 /sys/kernel/mm/transparent_hugepage/hugepages-xxxKB/shmem_enabled，
如果是 shm tmpfs，就是获得 mount -o huge=xxx 参数。最后两者都会调用
shmem_suitable_orders() 判断没有超过 vma 范围+起始地址对齐等，获得合适 order。

我们就可以根据用户想要 mTHP size 进行分配内存，shm_anon 如: 用户设置
hugepages-64KB/shmem_enabled 为 always，并且 64KB 没有超过 vma 范围，同时
起始地址对齐到 64KB，那么最终就返回 order = 4 来分配 64KB large folio。
shm_tmpfs 如: 用户设置 mount -o huge=always，返回 `[8KB, 2MB]` 来分配 large folio。

```bash
$ echo always > /sys/kernel/mm/transparent_hugepage/hugepages-64kB/shmem_enabled
$
static void test_shm_anon(void)
{
	int size = 1024 * 1024 * 1024; // 1G
	char *buf;
	int fd;

	fd = memfd_create("shm_anon", 0);
	ftruncate(fd, size);

	buf = mmap((void *)0x7fff00000000, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memset(buf, 'A', size);

	munmap(buf, size);
	close(fd);
}
```

以上 demo 分配 1G 共享匿名页，同时将 mTHP 64KB 设置为 always，通过
`perf stat -e page-faults,cache-misses,cache-references -- ./a.out` 获得
pagefault次数、cache-misses、cache-references，可知使用mTHP 64KB 后，
pagefaul次数从 262170 下降到 16410，cache-misses 从 2.77% 下降到 1.72%

```bash
$ mount -o remount,huge=always /dev/shm
$
static void test_shm_posix(void)
{
	int size = 1024 * 1024 * 1024; // 1G
	char *buf;
	int fd;

	fd = shm_open("shm_posix", O_RDWR | O_CREAT, 0777);
	ftruncate(fd, size);

	buf = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	memset(buf, 'A', size);

	munmap(buf, size);
	shm_unlink("shm_posix");
}
```

以上 demo 在 shm tmpfs 分配 1G 文件，同时将 tmpfs huge=always，通过
`perf stat -e page-faults,cache-misses,cache-references -- ./a.out` 获得
pagefault次数、cache-misses、cache-references，可知使用mTHP后，pagefaul次数
从 262170 下降到 538，cache-misses 从 3.27% 下降到 2.87%

```bash
$ mount -t tmpfs -o size=1G,huge=always tmpfs ~/foo
$ dd if=/dev/urandom of=~/foo/testfile bs=1M count=1024
```

以上 demo 在 shm tmpfs 分配 1G 文件，同时将 tmpfs huge=always，通过
`perf stat -e page-faults,cache-misses,cache-references -- ./a.out` 获得
pagefault次数、cache-misses、cache-references，可知使用mTHP后，cache-misses
从 13.06% 下降到 6.22%。

为什么 pagefault 次数没有变化？默认 pagefault 次数也只有 339 次，因为 shm tmpfs
走文件页路径，文件页默认有 readahead 功能，所以 pagefaults 没有变化。 设置
huge=always 前，内核都是 4KB 分配，所以 TBL  misses 较大。设置 huge=always
后，触发 pagefaults 后，一次分配内存都是在 `[8KB, 2MB]` 区间，能够将多个 pte entry
合并为 一个 TLB entry，所以 cache-misses 下降了。

为什么 shm tmpfs 是从 `[8KB, 2MB]` 区间分配内存，而不是像前面从一个固定大小分配内存。
由于文件系统 large folio 功能都是自动从文件内容大小选择 size，tmpfs huge= 只支持
always/madvise/never，没有颗粒度大小选择。

## 四、未来 feature

前面介绍 mTHP（匿名内存、共享匿名内存 shm_anon、shm_tmpfs）通过缺页异常来直接
分配 large folio ，目前这个这些特性都在 upstream 内核已合并，已支持。

[khugepaged: mTHP support](https://lore.kernel.org/linux-mm/20250912032810.197475-1-npache@redhat.com/) (未合并)
内核线程 khugepaged 在后台自动将多个 page 合并为一个 large folio

主要原理：

khugepaged 默认只 collapse PMD 大小，当 khugepaged 支持 mTHP 后，使用bitmap 记录
PMD 范围内每个 page 的占用状态（每个 page 对应一个 bit，1表示占用，0表示空闲），
通过对 bitmap 进行二分递归，从 PMD_ORDER 开始逐级向下寻找最优mTHP 大小，从而进行
collapse

## 参考

https://zhuanlan.zhihu.com/p/23703525187
https://linuxkernel.org.cn/doc/html/latest/admin-guide/mm/transhuge.html

