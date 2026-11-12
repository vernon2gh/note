# 简介

proc - process information pseudo-filesystem

关于详细说明解释，如下：

```
$ vim Documentation/filesystems/proc.rst
or
$ man proc
```

在proc文件系统中有对每个进程维护一个目录/proc/[pid]/，其中`/proc/self` 指向 打开此文件的进程

# Memory

`/proc/[pid]/pagemap`文件展示了该进程的物理帧与虚拟页的映射关系

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

`/proc/meminfo` 提供内存信息

> Documentation/filesystems/proc.rst

```
MemTotal      Total usable RAM (i.e. physical RAM minus a few reserved
              bits and the kernel binary code)
MemFree       Total free RAM.
MemAvailable  An estimate of how much memory is available for starting new
              applications, without swapping. Calculated from MemFree,
              SReclaimable, the size of the file LRU lists, and the low
              watermarks in each zone.

## slab allocator
Slab          slab 分配器分配给内核空间使用的总内存使用量，包含 SReclaimable + SUnreclaim
SReclaimable  在内存压力大时，能够被回收的 slab 内存使用量。
              如：kmalloc(__GFP_RECLAIMABLE), kmem_cache_create(SLAB_RECLAIM_ACCOUNT)
SUnreclaim    在内存压力大时，不能被回收的 slab 内存使用量。
              如：默认 kmalloc(), kmem_cache_create()

CommitLimit   目前系统能够被申请的虚拟内存总大小
Committed_AS  目前系统已经申请成功的虚拟内存大小
```

`/proc/vmstat` 统计内存使用次数

```
## LRU list
pgactivate              active list 页数
pgdeactivate            inactive list 页数

## pagefault
## minorfault           不需要从磁盘读数据的 pagefault，如 匿名页
## majorfault           需要从磁盘读数据的 pagefault，如 文件页、有 swapfile 的匿名页
## task->min_flt        某个进程的 minorfault 次数
## task->maj_flt        某个进程的 majorfault 次数
pgfault                 整个系统发生 pagefault 的总次数，包括 minorfault + majorfault。
pgmajfault              整个系统发生 majorfault 的总次数

## 内存回收，通过 kswapd, direct reclaim and khugepaged 进行内存回收
pgscan_kswapd           内存回收时，通过 kswapd 扫描的页数
pgscan_direct           内存回收时，通过 direct reclaim 扫描的页数
pgscan_direct_throttle  代表进入直接回收内存路径，但是没有进行直接回收内存的次数。
                        比如：因为当前进程已经收到 SIGKILL 信号，马上会被杀掉了，
                        所以即使当前 OOM 也无所谓。
pgscan_khugepaged       内存回收时，通过 khugepaged 扫描的页数
pgsteal_kswapd          内存回收时，通过 kswapd 成功回收的页数
pgsteal_direct          内存回收时，通过 direct reclaim 成功回收的页数
pgsteal_khugepaged      内存回收时，通过 khugepaged 成功回收的页数

## 内存回收，匿名页与文件页
pgscan_anon             内存回收时，扫描的匿名页数
pgscan_file             内存回收时，扫描的文件页数
pgsteal_anon            内存回收时，成功回收的匿名页数
pgsteal_file            内存回收时，成功回收的文件页数

## page out/in
pgpgout                 匿名页/文件页被回写到磁盘的总大小，单位 1KB
pgpgin                  从磁盘读取匿名页/文件页的总大小，单位 1KB
pswpout                 匿名页被回写到 swapfile/zram 的页数，单位 4KB
pswpin                  从 swapfile/zram 读取匿名页的页数，单位 4KB
zswpout                 匿名页被回写到 zswap 的次数
zswpin                  从 zswap 读取匿名页的次数

## workingset
workingset_refault_anon  之前回收的匿名页，再一次触发 pagefault 的次数
workingset_refault_file  之前回收的文件页，再一次触发 pagefault 的次数
workingset_activate_anon 之前回收的匿名页，马上立刻再一次触发 pagefault 的次数。
workingset_activate_file 之前回收的文件页，马上立刻再一次触发 pagefault 的次数。
workingset_restore_anon  之前回收的匿名页（位于 active workingset），
                         马上立刻再一次触发 pagefault 的次数
workingset_restore_file  之前回收的文件页（位于 active workingset），
                         马上立刻再一次触发 pagefault 的次数
workingset_nodereclaim   shadow node 被回收的次数

## other
zone_reclaim_failed      没有回收到所需页数量的次数
allocstall_xxx           自从开机以来，调用直接回收内存的次数
pageoutrun               自从开机以来，调用 kswapd 回收内存的次数
```

`/proc/buddyinfo` 显示 linux kernel buddy 分配器的分布情况

```bash
$ cat /proc/buddyinfo
Node 0, zone      DMA      0      0      0      0      0      0      0      0      1      2      2
Node 0, zone    DMA32   3480   3433   2595   1876   1386    692    566    328    154    102    220
Node 0, zone   Normal   1424  20607  43125  20880   5796   4854   1420    448    209    151    183
```

如下，整个系统有一个 Node 0，三个 zone（DMA、DMA32、Normal），其中
Normal zone 有 1424 个 order 0 的物理页，20607 个 order 1 的物理页，
43125 个 order 2 的物理页 ... 183 个 order 10 的物理页。

`/proc/pagetypeinfo`，对 `/proc/buddyinfo` 的进一步详细解析

```bash
$ sudo cat /proc/pagetypeinfo
Page block order: 9
Pages per block:  512

Free pages count per migrate type at order       0      1      2      3      4      5      6      7      8      9     10
Node    0, zone      DMA, type    Unmovable      0      0      0      0      0      0      0      0      1      1      0
Node    0, zone      DMA, type      Movable      0      0      0      0      0      0      0      0      0      1      2
Node    0, zone      DMA, type  Reclaimable      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone      DMA, type   HighAtomic      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone      DMA, type      Isolate      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone    DMA32, type    Unmovable    116    163    133     87     53     15      7      3      1      0      1
Node    0, zone    DMA32, type      Movable   3208   3182   2405   1752   1309    660    546    318    152    101    219
Node    0, zone    DMA32, type  Reclaimable    156     88     57     37     24     17     13      7      1      1      0
Node    0, zone    DMA32, type   HighAtomic      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone    DMA32, type      Isolate      0      0      0      0      0      0      0      0      0      0      0
Node    0, zone   Normal, type    Unmovable    207   1355   1095    331    133     12      0      0      0      0      0
Node    0, zone   Normal, type      Movable    391  18682  41829  20511   5645   4836   1416    445    208    151    183
Node    0, zone   Normal, type  Reclaimable   1394    608    214     54     17      6      4      3      1      0      0
Node    0, zone   Normal, type   HighAtomic     40     27     15      7      1      0      0      0      0      0      0
Node    0, zone   Normal, type      Isolate      0      0      0      0      0      0      0      0      0      0      0

Number of blocks type     Unmovable      Movable  Reclaimable   HighAtomic      Isolate
Node 0, zone      DMA            3            5            0            0            0
Node 0, zone    DMA32           14         1096           18            0            0
Node 0, zone   Normal          170         6460          392            2            0
```

如上，Normal zone 的 Movable page 有 391 个 order 0 的物理页，
18682 个 order 1 的物理页，41829 个 order 2 的物理页 ... 183 个 order 10 的物理页。

page block order 等于 9，每一个 page block 有 512 个物理页，其中
Normal zone 的 Movable page 总共有 6460 个 page block。

`/proc/zoneinfo`        内存区域使用情况

每一个 zone 区域的 min/low/high 水位值，如下：

```
$ cat /proc/zoneinfo | grep -E "Node |min |low |high "
Node 0, zone      DMA
        min      15
        low      18
        high     21
Node 0, zone    DMA32
        min      2323
        low      2903
        high     3483
Node 0, zone   Normal
        min      14556
        low      18195
        high     21834
Node 0, zone  Movable
        min      0
        low      0
        high     0
Node 0, zone   Device
        min      0
        low      0
        high     0
```

`/proc/slabinfo`

`/proc/vmallocinfo`     虚拟内存分配信息

`/proc/swaps`           swap分区使用情况

`/proc/mtd`             内存设备分区表信息

`/proc/dma`             DMA（直接内存访问）通道的列表

`/proc/mtrr`            系统使用的Memory Type Range Registers (MTRRs)

`/proc/kpagecount`

`/proc/kpageflags`

# IO

`/proc/diskstats` 显示块设备的 I/O 统计信息

```bash
$ cat /proc/diskstats
8   0 sda 370019 40096 86224849 3078088 134149 90231 115055594 3758714 0 1515684 6992806 0 0 0 0 36145 156004
8   1 sda1 26 0 208 391 0 0 0 0 0 400 391 0 0 0 0 0 0
8   2 sda2 185 23 11473 1324 3 0 10 1 0 932 1325 0 0 0 0 0 0
8   3 sda3 369697 40073 86208752 3075308 134146 90231 115055584 3758713 0 1515332 6834021 0 0 0 0 0 0
```

* 第 1 列，代表 major number
* 第 2 列，代表 minor number
* 第 3 列，代表 device name

* 第 4 列，代表 reads completed successfully
* 第 5 列，代表 reads merged
* 第 6 列，代表 sectors read
* 第 7 列，代表 time spent reading (ms)

* 第 8 列，代表 writes completed
* 第 9 列，代表 writes merged
* 第 10 列，代表 sectors written
* 第 11 列，代表 time spent writing (ms)

* 第 12 列，代表 I/Os currently in progress
* 第 13 列，代表 time spent doing I/Os (ms)
* 第 14 列，代表 weighted time spent doing I/Os (ms)

* 第 15 列，代表 discards completed successfully
* 第 16 列，代表 discards merged
* 第 17 列，代表 sectors discarded
* 第 18 列，代表 time spent discarding

* 第 19 列，代表 flush requests completed successfully
* 第 20 列，代表 time spent flushing

`/proc/filesystems`     目前系统支持的文件系统

`/proc/ioports`         当前系统硬件设备使用的IO端口列表

`/proc/iomem`           I/O 内存映射

`/proc/locks`           当前被内核锁定的文件

`/proc/mounts`          当前挂载信息

# CPU

`/proc/cpuinfo`         cpu相关信息

`/proc/loadavg`         当前系统负载

`/proc/softirqs`        系统软中断信息

`/proc/schedstat`       调度器信息

`/proc/sched_debug`     调度器debug信息

# Kernel

`/proc/cmdline`         在引导启动时传递给Linux内核的参数

`/proc/crypto`          内核支持的加密方式

`/proc/modules`         当前系统已经加载的模块（lsmod）

`/proc/version`         内核版本信息

`/proc/stat`            系统和内核的统计信息

`/proc/fb`              内核编译期间帧缓冲信息

`/proc/kmsg`            内核日志信息

`/proc/kcore`           表示系统物理内存，可以用gdb检查内核数据结构的当前状态

`/proc/kallsyms`        内核符号信息，主要用于调试

`/proc/timer_list`      内核各种计时器信息

`/proc/timer_stats`

`/proc/sysrq-trigger`   内核触发器（危险！！！）

`/proc/execdomains`     Linux内核当前支持的execution domains

# Other

`/proc/[pid]/stat`文件展示了该进程的状态

```bash
$ cat /proc/1/stat
1 (init) S 0 1 1 0 -1 4210944 53 5545 19 9 2 133 111 68 20 0 1 0 83 7581696 403 18446744073709551615 94829180252160 94829180995132 140729949788528 0 0 0 0 0 537414151 1 0 0 17 0 0 0 17 0 0 94829183095120 94829183107699 94829195890688 140729949790160 140729949790171 140729949790171 140729949790189 0
```

* 第1列, 表示 进程的PID
* 第2列, 表示 进程的名称
* 第3列, 表示 进程的状态(S表示Sleep)
* 第4列, 表示 进程的PPID，即父进程的PID
* ...
* 第15列, 表示 进程在内核空间 running 的时间
* ...
* 第41列, 表示 进程调度策略(0: TS, 1: FF)
* ...

`/proc/interrupts`      中断表

`/proc/uptime`          系统运行时间

`/proc/devices`         设备信息（主设备号等）

`/proc/mdstat`          虚拟设备信息（软raid等）

`/proc/misc`            其他的主要设备(设备号为10)上注册的驱动

`/proc/cgroup`          cgroup相关信息

`/proc/consoles`
