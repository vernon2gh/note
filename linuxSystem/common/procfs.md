## 0. 简介

proc - process information pseudo-filesystem

关于详细说明解释，如下：

```
$ man proc
```

在proc文件系统中有对每个进程维护一个目录/proc/[pid]/，其中`/proc/self` 指向 打开此文件的进程

### 1. stat

`/proc/[pid]/stat`文件展示了该进程的状态

```bash
$ cat /proc/1/stat
1 (init) S 0 1 1 0 -1 4210944 53 5545 19 9 2 133 111 68 20 0 1 0 83 7581696 403 18446744073709551615 94829180252160 94829180995132 140729949788528 0 0 0 0 0 537414151 1 0 0 17 0 0 0 17 0 0 94829183095120 94829183107699 94829195890688 140729949790160 140729949790171 140729949790171 140729949790189 0
```

对/proc/[pid]/stat的解释如下：

* 第1列, 表示 进程的PID
* 第2列, 表示 进程的名称
* 第3列, 表示 进程的状态(S表示Sleep)
* 第4列, 表示 进程的PPID，即父进程的PID
* ……
* 第41列, 表示 进程调度策略(0: TS, 1: FF)
* ....

/proc/[pid]/stat的输出内容，在linux源码中fs/proc/array.c中设置，如下：

```bash
## based on linux 5.4 version
$ vim fs/proc/array.c
static int do_task_stat(struct seq_file *m, struct pid_namespace *ns,
			struct pid *pid, struct task_struct *task, int whole)
{
	....
	seq_put_decimal_ull(m, "", pid_nr_ns(pid, ns)); // 进程的PID
	seq_puts(m, " (");
	proc_task_name(m, task, false);                 // 进程的名称
	seq_puts(m, ") ");
	seq_putc(m, state);                             // 进程的状态
	seq_put_decimal_ll(m, " ", ppid);               // 进程的PPID
	...
	seq_put_decimal_ull(m, " ", task->policy);      // 进程调度策略
	...
}
```

### 2. pagemap

`/proc/[pid]/pagemap`文件展示了该进程的 物理帧与虚拟页的映射关系

```
## Documentation/vm/pagemap.txt

    ...
    * Bits 0-54  page frame number (PFN) if present
    * Bits 0-4   swap type if swapped
    * Bits 5-54  swap offset if swapped
    * Bit  55    pte is soft-dirty (see Documentation/vm/soft-dirty.txt)
    * Bit  56    page exclusively mapped (since 4.2)
    * Bits 57-60 zero
    * Bit  61    page is file-page or shared-anon (since 3.5)
    * Bit  62    page swapped
    * Bit  63    page present
    ...
```

3. meminfo

`/proc/meminfo` 提供有关内存分布和利用率的信息

```
## Documentation/filesystems/proc.rst

MemTotal      Total usable RAM (i.e. physical RAM minus a few reserved
              bits and the kernel binary code)
MemFree       Total free RAM.
MemAvailable  An estimate of how much memory is available for starting new
              applications, without swapping. Calculated from MemFree,
              SReclaimable, the size of the file LRU lists, and the low
              watermarks in each zone.
Slab          in-kernel data structures cache
SReclaimable  Part of Slab, that might be reclaimed, such as caches
SUnreclaim    Part of Slab, that cannot be reclaimed on memory pressure
```

